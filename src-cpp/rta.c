

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
#define JUST_CHA 0


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

/*
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
*/

typedef struct callgraph_walker_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpmap_t *entity2graph; // map for finding the graph to a given entity
	cpmap_t *vtable2class; // map for finding the class to a given vtable entity
	cpset_t *used_classes; // used classes found by examining object creation or coming in from external functions
	cpset_t *used_methods; // used method entities
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically linked call (Map: call entity -> Set: method entities)
	cpmap_t *disabled_targets; // map that stores the set of disabled potential call targets of dynamic calls for every class) (Map: class -> Set: method entities)
} callgraph_walker_env;


static void add_to_workqueue(ir_entity *method, callgraph_walker_env *env); // forward declaration


// add method entity to targets set of all matching calls
// /!\ should normally be called with call_entity and method entity the same to start search from the method entity you have
// searches upwards in the class hierarchy if there were calls of overwritten methods
static void add_to_dyncalls(ir_entity *call_entity, ir_entity *method, callgraph_walker_env *env) {
	assert(is_method_entity(call_entity));
	assert(is_method_entity(method));
	assert(env);

	cpset_t *targets = cpmap_find(env->dyncall_targets, call_entity);
	if (targets != NULL && cpset_find(targets, method) == NULL) { // if there were calls to this entity AND method not already added to targets set
		printf("\t\t\t\t\tupdating method %s.%s for call %s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity));
		// add to targets set
		cpset_insert(targets, method);

		// add to used methods
		cpset_insert(env->used_methods, method);

		// add to workqueue
		add_to_workqueue(method, env);
	}

	// search recursively upwards if there were calls to overwritten entities (and if found add the method entity to their targets set)
	for (size_t i=0; i<get_entity_n_overwrites(call_entity); i++) {
		ir_entity *overwritten_method = get_entity_overwrites(call_entity, i);
		assert(overwritten_method != call_entity);
		add_to_dyncalls(overwritten_method, method, env);
	}
}

static void add_new_used_class(ir_type *klass, callgraph_walker_env *env) {
	assert(is_Class_type(klass));
	assert(env);

	if (cpset_find(env->used_classes, klass) == NULL) { // if it had not already been added
		// add to used classes
		cpset_insert(env->used_classes, klass);
		printf("\t\t\t\t\tadded new used class %s\n", get_class_name(klass));

		// update existing results
		cpset_t *methods = cpmap_find(env->disabled_targets, klass);
		if (methods != NULL) {
			cpset_iterator_t iterator;
			cpset_iterator_init(&iterator, methods);
			ir_entity *method;
			printf("\t\t\t\t\tup to %u methods to enable\n", cpset_size(methods));
			while ((method = cpset_iterator_next(&iterator)) != NULL) {
				add_to_dyncalls(method, method, env); // search recursively upwards if overwritten entities had been called and add it to their target sets
				cpset_remove_iterator(methods, &iterator);
			}
		}
	}
}

static void memorize_disabled_method(ir_type *klass, ir_entity *entity, callgraph_walker_env *env) {
	assert(is_Class_type(klass));
	assert(is_method_entity(entity));
	assert(env);

	cpset_t *methods = cpmap_find(env->disabled_targets, klass);
	if (methods == NULL) {
		methods = new_cpset(hash_ptr, ptr_equals);
		cpmap_set(env->disabled_targets, klass, methods);
	}
	cpset_insert(methods, entity);
}

static void add_all_subclasses(ir_type *klass, callgraph_walker_env *env) {
	assert(is_Class_type(klass));
	assert(env);

	// add itself
	add_new_used_class(klass, env);

	printf("\t\t\tadded class type %s to used types\n", get_class_name(klass));

	// add recursively all subclasses
	for (size_t i=0; i<get_class_n_subtypes(klass); i++) {
		ir_type *subclass = get_class_subtype(klass, i);
		add_all_subclasses(subclass, env);
	}
}

static void handle_external_method(ir_entity *method, callgraph_walker_env *env) { //TODO
	assert(is_method_entity(method));
	assert(env);

	printf("\t\thandling external method %s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
	//TODO what if we already had handled this external method?
	ir_type *methodtype = get_entity_type(method);
	for (size_t i=0; i<get_method_n_ress(methodtype); i++) {
		ir_type *type = get_method_res_type(methodtype, i);
		printf("\t\t\tresult type %s\n", gdb_node_helper(type));
		if (is_Pointer_type(type)) {
			ir_type *targettype = get_pointer_points_to_type(type);
			if (is_Class_type(targettype)) {
				// add class and all its subclasses to used types
				//TODO what with interfaces? or rather how to handle interfaces? or are they just class types??
				add_all_subclasses(targettype, env);
			}
		}
	}
}

static void add_to_workqueue(ir_entity *method, callgraph_walker_env *env) {
	assert(is_method_entity(method));
	assert(env);

	ir_graph *graph = (ir_graph*)cpmap_find(env->entity2graph, method);
	if (graph) {
		//if (cpset_find(env->done_set, graph) == NULL) { // only enqueue if not already done yet
			printf("\t\tadding %s.%s to workqueue\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			pdeq_putr(env->workqueue, graph);
		//}
	} else {
		// treat methods without graph as external methods
		// since we can't analyze it, mark all potentially returned object types as in use (completely down the class hierarchy!)
		printf("\t\tfound no graph, probably external\n");
		handle_external_method(method, env);
	}
}

static void take_entity(ir_entity *entity, cpset_t *result_set, callgraph_walker_env *env) {
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	if (cpset_find(result_set, entity) == NULL) { // take each entity only once (the sets won't mind but the workqueue)
		printf("\t\ttaking entity %s.%s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity));

		// add to used
		cpset_insert(env->used_methods, entity);

		// add to result set
		cpset_insert(result_set, entity);

		// add to workqueue
		add_to_workqueue(entity, env);
	}
}

// collect method entities from downwards in the class hierarchy
// /!\ should normally be called with the owner class of the entity e.g. to start at the static looked up entity
// it walks down the classes to have the entities with the classes even when the method is inherited
static void collect_methods(ir_type *klass, ir_entity *entity, cpset_t *result_set, callgraph_walker_env *env) {
	assert(is_Class_type(klass));
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	printf("\t\twalking class %s\n", get_class_name(klass));
	ir_entity *current_entity = entity;
	ir_entity *overwriting_entity = get_class_member_by_name(klass, get_entity_ident(current_entity)); //?? Is there a more efficient way?
	if (overwriting_entity != NULL && overwriting_entity != current_entity) { // if has overwriting entity
		assert(klass == get_entity_owner(overwriting_entity));
		printf("\t\t%s.%s overwrites %s.%s\n", get_class_name(get_entity_owner(overwriting_entity)), get_entity_name(overwriting_entity), get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity));

		current_entity = overwriting_entity;
	}
	//else // inherited

	if (JUST_CHA || cpset_find(env->used_classes, klass) != NULL) { // if in use
		take_entity(current_entity, result_set, env);
	} else {
		memorize_disabled_method(klass, current_entity, env); // remember entity with this class for patching if this class will become used
	}

	for (size_t i=0; i<get_class_n_subtypes(klass); i++) {
		ir_type *subclass = get_class_subtype(klass, i);

		collect_methods(subclass, current_entity, result_set, env);
	}
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
				printf("\t\t\t\tLHS: %s -> %s -> owner: %s\n", gdb_node_helper(lhs), get_entity_name(vtable_entity), get_class_name(get_entity_owner(vtable_entity)));
				ir_type *klass = cpmap_find(env->vtable2class, vtable_entity);
				assert(is_Class_type(klass));
				printf("\t\t\t\t\t-> vtable2class: %s\n", get_class_name(klass));

				add_new_used_class(klass, env);

				printf("\t\t\t\tRHS: %s\n", gdb_node_helper(get_irn_n(src, 1))); // rhs
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

			// add to used
			cpset_insert(env->used_methods, entity);

			add_to_workqueue(entity, env);
		} else if (is_Sel(fp)) {
			// handle dynamic call
			ir_entity *entity = get_Sel_entity(fp);
			printf("\tdynamic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

			if (cpmap_find(env->dyncall_targets, entity) == NULL) { // if not already done
				// calculate set of method entities that this call could potentially call

				// static lookup upwards in the class hierarchy (gets just one method entity)
				// The entity from the Sel node is already what the result of a static lookup would be.

				// collect all potentially called method entities from downwards the class hierarchy
				cpset_t *result_set = new_cpset(hash_ptr, ptr_equals);
				ir_type *owner = get_entity_owner(entity);
				collect_methods(owner, entity, result_set, env);
				assert(cpset_size(result_set) > 0);

				//printf("cpmap_insert dyncall_targets %x %x\n", (unsigned)entity,  (unsigned)result_set);
				cpmap_set(env->dyncall_targets, entity, result_set); //??
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
		//printf(" %s -> %s\n", get_entity_name(vtable_entity), gdb_node_helper(clss));
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
	cpset_t *used_classes = new_cpset(hash_ptr, ptr_equals);
	cpset_t *used_methods = new_cpset(hash_ptr, ptr_equals);
	cpmap_t *dyncall_targets = new_cpmap(hash_ptr, ptr_equals); assert(cpmap_size(dyncall_targets) == 0);
	cpmap_t *disabled_targets = new_cpmap(hash_ptr, ptr_equals);
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

	//callgraph_walker_env env = { workqueue, entity2graph, vtable2class, used_classes, ... };
	callgraph_walker_env env = {
		.workqueue = workqueue,
		.entity2graph = entity2graph,
		.vtable2class = vtable2class,
		.used_classes = used_classes,
		.used_methods = used_methods,
		.dyncall_targets = dyncall_targets,
		.disabled_targets = disabled_targets
	};

	cpset_t *done_set = new_cpset(hash_ptr, ptr_equals);
	while (!pdeq_empty(workqueue)) {
		ir_graph *g = (ir_graph*)pdeq_getl(workqueue);
		assert(is_ir_graph(g));

		if (cpset_find(done_set, g) != NULL) continue;

		printf("\n== %s.%s\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)));

		cpset_insert(done_set, g); // mark as done _before_ walking because of possible recursive calls !??? -> not necessary but not wrong!? TODO
		irg_walk_graph(g, NULL, callgraph_walker, &env);
	}

	printf("\n\n==== Results ==============================================\n");
	{
		printf("\nused classes:\n");
		cpset_iterator_t iterator;
		cpset_iterator_init(&iterator, used_classes);
		ir_type *klass;
		while ((klass = cpset_iterator_next(&iterator)) != NULL) {
			printf("\t%s\n", get_class_name(klass));
		}
	}
	{
		printf("\nused methods:\n");
		cpset_iterator_t iterator;
		cpset_iterator_init(&iterator, used_methods);
		ir_entity *method;
		while ((method = cpset_iterator_next(&iterator)) != NULL) {
			//printf("\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
			printf("\t%s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
		}
	}
	{
		printf("\ndyncall targets:\n");
		//printf("size %u\n", cpmap_size(dyncall_targets));
		cpmap_iterator_t iterator;
		cpmap_iterator_init(&iterator, dyncall_targets);
		cpmap_entry_t *entry;
		while ((entry = cpmap_iterator_next(&iterator)) != NULL) {
			const ir_entity *call_entity = entry->key;
			assert(call_entity);
			//printf("\t%s.%s %s\n", get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity), gdb_node_helper(call_entity));
			printf("\t%s.%s\n", get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity));

			cpset_t *targets = entry->data;
			assert(targets);
			cpset_iterator_t it;
			cpset_iterator_init(&it, targets);
			ir_entity *method;
			while ((method = cpset_iterator_next(&it)) != NULL) {
				//printf("\t\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
				printf("\t\t%s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			}
		}
	}

	// free data structures
	//TODO
}
