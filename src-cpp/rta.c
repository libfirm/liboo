

#include "include/liboo/rta.h"

#include <assert.h>

#include <stdbool.h>

#include "adt/cpmap.h"
#include "adt/cpset.h"
#include "adt/pdeq.h"
#include "adt/hashptr.h"


// override option just for early development to keep going without information about used types
#define JUST_CHA 1


static int ptr_equals(const void *pt1, const void *pt2) { // missing default pointer compare function
	return pt1 == pt2;
}

static inline cpmap_t *new_cpmap(cpmap_hash_function hash_function, cpmap_cmp_function cmp_function) { // missing new function for cpmap
	cpmap_t *cpmap = (cpmap_t*)malloc(sizeof(cpmap_t));
	cpmap_init(cpmap, hash_function, cmp_function);
	return cpmap;
}

static inline cpset_t *new_cpset(cpset_hash_function hash_function, cpset_cmp_function cmp_function) { // missing new function for cpmap
	cpset_t *cpset = (cpset_t*)malloc(sizeof(cpset_t));
	cpset_init(cpset, hash_function, cmp_function);
	return cpset;
}

static inline cpmap_iterator_t *new_cpmap_iterator(cpmap_t *map) { // missing new function for cpmap iterator
	cpmap_iterator_t *it = (cpmap_iterator_t*)malloc(sizeof(cpmap_iterator_t));
	cpmap_iterator_init(it, map);
	return it;
}

static inline cpset_iterator_t *new_cpset_iterator(cpset_t *set) { // missing new function for cpset iterator
	cpset_iterator_t *it = (cpset_iterator_t*)malloc(sizeof(cpset_iterator_t));
	cpset_iterator_init(it, set);
	return it;
}


typedef struct callgraph_walker_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpmap_t *entity2graph; // map for finding the graph to a given entity
	cpset_t *enabled_types_set; // used types found in object creation or coming in from external functions
	cpset_t *unhandled_dyncalls_set; // unhandled potential call targets of dynamic calls
} callgraph_walker_env;

static void move_enabled_methods_from_unhandled_to_workqueue(callgraph_walker_env *env) {
	assert(env);
	cpset_t *unhandled_methods = env->unhandled_dyncalls_set;

	cpset_iterator_t *it = new_cpset_iterator(unhandled_methods);
	ir_entity *entity;
	while ((entity = cpset_iterator_next(it)) != NULL) {
		//ir_entity *entity = (ir_entity*)entry->key;
		ir_type *klass = get_entity_owner(entity);
		assert(is_Class_type(klass));
		if (cpset_find(env->enabled_types_set, klass) != NULL) {
			ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, entity);
			assert(graph);
			pdeq_putr(env->workqueue, graph);
			cpset_remove_iterator(unhandled_methods, it);
		}
	}
}

static void add_all_subclasses_to_set(cpset_t *set, ir_type *klass, callgraph_walker_env *env) {
	assert(set);
	assert(is_Class_type(klass));
	assert(env);

	printf("\t\t\tadded class type %s to used types\n", get_class_name(klass));

	// add itself
	cpset_insert(set, klass);

	// look through previously unhandled methods and move those that are now enabled to worklist (removing them from the unhandled methods set)
	move_enabled_methods_from_unhandled_to_workqueue(env);

	// add recursively all subclasses
	for (size_t i=0; i<get_class_n_subtypes(klass); i++) {
		ir_type *subclass = get_class_subtype(klass, i);
		add_all_subclasses_to_set(set, subclass, env);
	}
}

static void handle_external_method(ir_entity *entity, callgraph_walker_env *env) {
	assert(is_method_entity(entity));
	assert(env);

	ir_type *methodtype = get_entity_type(entity);
	printf("\t\tfound no graph, probably external\n");
	for (size_t i=0; i<get_method_n_ress(methodtype); i++) {
		ir_type *type = get_method_res_type(methodtype, i);
		printf("\t\t\t%s\n", gdb_node_helper(type));
		if (is_Pointer_type(type)) {
			ir_type *targettype = get_pointer_points_to_type(type);
			if (is_Class_type(targettype)) {
				// add class and all its subclasses to used types
				//TODO what with interfaces? or rather how to handle interfaces? or are they just class types??
				add_all_subclasses_to_set(env->enabled_types_set, targettype, env);
			}
		}
	}
}

static void add_overwriting_method(ir_entity *entity, callgraph_walker_env *env) {
	assert(is_method_entity(entity));
	assert(env);

	ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, entity);
	if (graph) {
		ir_type *klass = get_entity_owner(entity);
		assert(is_Class_type(klass));
		if (JUST_CHA || cpset_find(env->enabled_types_set, klass) != NULL) {
			assert(graph);
			pdeq_putr(env->workqueue, graph);
		} else {
			cpset_insert(env->unhandled_dyncalls_set, entity); // add to unhandled set
		}
		// add recursively all overwriting method entities
		for (size_t i=0; i<get_entity_n_overwrittenby(entity); i++) {
			ir_entity *ent = get_entity_overwrittenby(entity, i);
			printf("\t\t\t %s.%s %s overwritten", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
			printf(" by %s.%s %s\n", get_class_name(get_entity_owner(ent)), get_entity_name(ent), gdb_node_helper(ent));
			add_overwriting_method(ent, env);
		}
	}
	//else {} // not necessary to handle external method, done at top level
}

static void callgraph_walker(ir_node *node, void *environment) {
	assert(environment);
	callgraph_walker_env *env = (callgraph_walker_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Call: {
		ir_node *fp = get_irn_n(node, 1);
		if (is_SymConst(fp)) {
			// handle static call
			ir_entity *entity = get_SymConst_entity(fp);
			printf("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
			ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, entity);
			if (get_entity_visibility(entity) != ir_visibility_external && graph) { // ignore external methods and (if there are) other methods without an graph, especially calloc
					// treat methods without graph as external methods AND mark all potentially returned types as in use (all means completely down the class hierarchy!)
				pdeq_putr(env->workqueue, graph);
			} else {
				// can't analyze method
				// mark all potentially returned object types as in use
				handle_external_method(entity, env);
			}
			//TODO recognize constructor calls to see which types are used
			// /!\ but ignore constructor calls in constructor methods that have the this pointer as argument!!
			//if (is_constructor(...) && !is_constructor(current_method) && argument1 == current_argument1) {
			// ... get_constructed_type(

			//TODO if found new used type put all its methods in unhandled_dyncalls_set into workqueue !?
			move_enabled_methods_from_unhandled_to_workqueue(env);
		} else if (is_Sel(fp)) {
			// handle dynamic call
			ir_entity *entity = get_Sel_entity(fp);
			printf("\tdynamic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

			// static lookup upwards in the class hierarchy (gets just one method entity)
				// -> seems to be already done!?
/*			for (size_t i=0; i<get_entity_n_overwrites(entity); i++) {
				printf("\toverwrites: %s\n", gdb_node_helper(get_entity_overwrites(entity, i))); //?? docu comments might be switched!?
			}
*/
			// take it
			ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, entity);
			if (graph) {
				ir_type *klass = get_entity_owner(entity);
				assert(is_Class_type(klass));
				if (JUST_CHA || cpset_find(env->enabled_types_set, klass) != NULL) {
					pdeq_putr(env->workqueue, graph);
				} else {
					 // add to unhandled set
					cpset_insert(env->unhandled_dyncalls_set, entity);
				}
			} else {
				// can't analyze method
				// mark all potentially returned object types as in use
				handle_external_method(entity, env);
			}
			// take all from downwards in the class hierarchy
				//TODO search down the class hierarchy
			for (size_t i=0; i<get_entity_n_overwrittenby(entity); i++) {
				ir_entity *ent = get_entity_overwrittenby(entity, i); //?? docu comments might be switched!?

				printf("\t\toverwrittenby: %s.%s %s\n", get_class_name(get_entity_owner(ent)), get_entity_name(ent), gdb_node_helper(ent));

				add_overwriting_method(ent, env);
			}
		}
		else
			assert(false); // neither SymConst nor Sel as callee shouldn't happen!?
		break;
	}
	}
}

void run_rta(ir_entity *javamain) { //TODO for other programming languages we might also need to analyze something like global constructors! What's with static sections in Java? (This is important because of closed world assumption!)
	assert(javamain);

	cpmap_t *entity2graph = new_cpmap(hash_ptr, ptr_equals);

	for (size_t i = 0; i<get_irp_n_irgs(); i++) {
		ir_graph *g = get_irp_irg(i);
		cpmap_set(entity2graph, get_irg_entity(g), g);
	}

	//TODO refactor
	cpset_t *enabled_types_set = new_cpset(hash_ptr, ptr_equals);
	cpset_t *unhandled_dyncalls_set = new_cpset(hash_ptr, ptr_equals);
	pdeq *workqueue = new_pdeq();
	pdeq_putr(workqueue, (ir_graph*)cpmap_find(entity2graph, javamain));

	//callgraph_walker_env env = { workqueue, entity2graph, enabled_types_set, unhandled_dyncalls_set };
	callgraph_walker_env env = { .workqueue = workqueue, .entity2graph = entity2graph, .enabled_types_set = enabled_types_set, .unhandled_dyncalls_set = unhandled_dyncalls_set };

	cpset_t *done_set = new_cpset(hash_ptr, ptr_equals);
	while (!pdeq_empty(workqueue)) {
		ir_graph *g = (ir_graph*)pdeq_getl(workqueue);
		assert(is_ir_graph(g));

		if (cpset_find(done_set, g) != NULL) continue;

		printf("\n== %s.%s\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)));

		cpset_insert(done_set, g); // mark as done _before_ walking because of possible recursive calls !??? -> not necessary but not wrong!? TODO
		irg_walk_graph(g, NULL, callgraph_walker, &env);
	}

/*	printf("\nunneeded dyncall methods:\n");
	pmap_entry *entry = pmap_first(unhandled_dyncalls_set);
	while (entry != NULL) {
		ir_entity *entity = (ir_entity*)entry->key;
		printf("\t%s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
		entry = pmap_next(unhandled_dyncalls_set);
	}
*/
	// free data structures
	//TODO
}
