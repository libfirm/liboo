

#include "include/liboo/rta.h"

#include <assert.h>

#include <stdbool.h>
#include <string.h>

#include <liboo/oo.h>

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
	cpmap_t *vtable2class; // map for finding the class to a given vtable entity
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

static void add_overwriting_methods(ir_entity *entity, callgraph_walker_env *env) {
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
			add_overwriting_methods(ent, env);
		}
	}
	//else {} // not necessary to handle external method, done at top level
}

static void callgraph_walker(ir_node *node, void *environment) {
	assert(environment);
	callgraph_walker_env *env = (callgraph_walker_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Store: {
		// look for store of vptrs to detect object creation
		ir_node *dest =  get_irn_n(node, 1);
		ir_node *src =  get_irn_n(node, 2);
		printf("\tstore: %s\n", gdb_node_helper(dest));
		if (is_Sel(dest)) {
			ir_entity *entity = get_Sel_entity(dest);
			printf("\t\t%s\n", get_entity_name(entity));
			if (strncmp(get_entity_name(entity), "@vptr", 5) == 0) { //?? check for vptr entity or are there more than one?
				printf("\t\t\t%s\n", gdb_node_helper(src));

				assert(is_Add(src)); // src is usually an Add node in bytecode2firm //TODO write code that correctly parses what liboo function ddispatch_prepare_new_instance creates according to the configured vtable layout
				ir_node *lhs = get_irn_n(src, 0);

				ir_entity *vtable_entity = get_SymConst_entity(lhs);
				printf("\t\t\t\t%s -> %s -> owner: %s\n", gdb_node_helper(lhs), get_entity_name(vtable_entity), get_class_name(get_entity_owner(vtable_entity)));
				ir_type *klass = cpmap_find(env->vtable2class, vtable_entity);
				assert(klass && is_Class_type(klass));
				printf("\t\t\t\t\t-> vtable2class: %s\n", get_class_name(klass));

				cpset_insert(env->enabled_types_set, klass);

				printf("\t\t\t\t%s\n", gdb_node_helper(get_irn_n(src, 1))); // rhs

				// if found new used type put all its methods in unhandled_dyncalls_set into workqueue !?
				move_enabled_methods_from_unhandled_to_workqueue(env); //TODO it would suffice to call this once each time the workqueue becomes empty!?
			}
		}
		break;
	}
	case iro_Call: {
		ir_node *fp = get_irn_n(node, 1);
		if (is_SymConst(fp)) {
			// handle static call
			ir_entity *entity = get_SymConst_entity(fp);
			printf("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
			ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, entity);
			if (graph) {
				pdeq_putr(env->workqueue, graph);
			} else { // treat methods without graph as external methods
				// can't analyze method
				// mark all potentially returned object types as in use (completely down the class hierarchy!)
				handle_external_method(entity, env);
			}
		} else if (is_Sel(fp)) {
			// handle dynamic call
			ir_entity *entity = get_Sel_entity(fp);
			printf("\tdynamic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

			// static lookup upwards in the class hierarchy (gets just one method entity)
			// The entity from the Sel node is already what the result of a static lookup would be.
/*			for (size_t i=0; i<get_entity_n_overwrites(entity); i++) {
				printf("\toverwrites: %s\n", gdb_node_helper(get_entity_overwrites(entity, i))); //?? docu comments might be switched!?
			}
*/

			// take it
			ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, entity);
			if (graph) {
				ir_type *klass = get_entity_owner(entity);
				assert(is_Class_type(klass));
				if (JUST_CHA || cpset_find(env->enabled_types_set, klass) != NULL) { //TODO what if inherited method and no objects of super class
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

				add_overwriting_methods(ent, env);
			}
		}
		else
			assert(false); // neither SymConst nor Sel as callee shouldn't happen!?
		break;
	}
	}
}


typedef struct class_walker_env {
	cpmap_t *vtable2class;
} class_walker_env;

static void class_walker(ir_type *clss, void* environment) {
	assert(environment);
	class_walker_env *env = (class_walker_env*)environment;

	cpmap_t *vtable2class = env->vtable2class;
	ir_entity *vtable_entity = oo_get_class_vtable_entity(clss);
	if (vtable_entity) {
		printf(" %s -> %s\n", get_entity_name(vtable_entity), gdb_node_helper(clss));
		cpmap_set(vtable2class, vtable_entity, clss);
	}
	else {
		//printf(" %s has no vtable\n", gdb_node_helper(clss));
	}
}

void run_rta(ir_entity *javamain) { //TODO for other programming languages we might also need to analyze something like global constructors! What's with static sections in Java? (This is important because of closed world assumption!)
	// TODO more parameters to give data structures for the results ?
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

	cpmap_t vtable2class_map;
	cpmap_t *vtable2class = &vtable2class_map;
	cpmap_init(vtable2class, hash_ptr, ptr_equals);
	// walk all classes to fill the vtable2class map
	{
		class_walker_env env = { .vtable2class = vtable2class };
		class_walk_super2sub(class_walker, NULL, &env);
	}

	//callgraph_walker_env env = { workqueue, entity2graph, vtable2class, enabled_types_set, unhandled_dyncalls_set };
	callgraph_walker_env env = { .workqueue = workqueue, .entity2graph = entity2graph, .vtable2class = vtable2class, .enabled_types_set = enabled_types_set, .unhandled_dyncalls_set = unhandled_dyncalls_set };

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
