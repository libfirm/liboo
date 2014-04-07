/*
 * This file is part of liboo.
 */

/**
 * @file	rta.c
 * @brief	Devirtualization of dynamically linked calls through Rapid Type Analysis
 * @author	Steffen Knoth
 * @date	2014
 */


#include "liboo/rta.h"

#include <assert.h>

#include <stdbool.h>
#include <string.h>

#include <liboo/oo.h>
#include <liboo/nodes.h>

#include "adt/cpmap.h"
#include "adt/cpset.h"
#include "adt/pdeq.h"
#include "adt/hashptr.h"


// override option just for early development to keep going without information about live classes
#define JUST_CHA 0


#define is_inherited_copy(X) (oo_get_method_is_inherited(X) && ddispatch_get_bound_entity(X) != X)


static ir_entity *get_class_member_by_name(ir_type *cls, ident *ident) { // function which was removed from newer libfirm versions
	for (size_t i = 0, n = get_class_n_members(cls); i < n; ++i) {
		ir_entity *entity = get_class_member(cls, i);
		if (get_entity_ident(entity) == ident)
			return entity;
	}
	return NULL;
}

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


static ir_type *default_detect_creation(ir_node *call) { (void)call; return NULL; }
static ir_entity *default_detect_call(ir_node *call) { (void)call; return NULL; }

static ir_type *(*detect_creation)(ir_node *call) = &default_detect_creation;
static ir_entity *(*detect_call)(ir_node *call) = &default_detect_call;

void rta_set_detection_callbacks(ir_type *(*detect_creation_callback)(ir_node *call), ir_entity *(*detect_call_callback)(ir_node *call)) {
	assert(detect_creation_callback);
	assert(detect_call_callback);
	detect_creation = detect_creation_callback;
	detect_call = detect_call_callback;
}


typedef struct analyzer_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpset_t *in_queue; // set to avoid duplicates in the workqueue
	cpset_t *done_set; // set to mark graphs that were already analyzed
	cpmap_t *vtable2class; // map for finding the class to a given vtable entity
	cpset_t *live_classes; // live classes found by examining object creation (external classes are left out and always considered as live)
	cpset_t *live_methods; // live method entities
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically linked call (Map: call entity -> Set: method entities)
	cpmap_t *disabled_targets; // map that stores the set of disabled potential call targets of dynamic calls for every class) (Map: class -> Set: method entities)
	cpset_t *external_methods; // set that stores already handled external methods (just for not handling the same method more than once)
} analyzer_env;


static void add_to_workqueue(ir_entity *method, analyzer_env *env); // forward declaration


// add method entity to targets set of all matching calls
// /!\ should normally be called with call_entity and method entity the same to start search from the method entity you have
// searches upwards in the class hierarchy if there were calls of overwritten methods
static void add_to_dyncalls(ir_entity *call_entity, ir_entity *method, analyzer_env *env) {
	assert(is_method_entity(call_entity));
	assert(is_method_entity(method));
	assert(env);

	cpset_t *targets = cpmap_find(env->dyncall_targets, call_entity);
	if (targets != NULL && cpset_find(targets, method) == NULL) { // if there were calls to this entity AND method not already added to targets set
		printf("\t\t\t\t\tupdating method %s.%s for call %s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity));
		// add to targets set
		cpset_insert(targets, method);

		// add to live methods
		cpset_insert(env->live_methods, method);

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

static void add_new_live_class(ir_type *klass, analyzer_env *env) {
	assert(is_Class_type(klass));
	assert(env);

	if (cpset_find(env->live_classes, klass) == NULL // if it had not already been added
	    && !oo_get_class_is_extern(klass) && !oo_get_class_is_abstract(klass)) { // if not extern and not abstract
		// add to live classes
		cpset_insert(env->live_classes, klass);
		printf("\t\t\t\t\tadded new live class %s\n", get_class_name(klass));

		// update existing results
		cpset_t *methods = cpmap_find(env->disabled_targets, klass);
		if (methods != NULL) {
			{
				cpset_iterator_t iterator;
				cpset_iterator_init(&iterator, methods);
				ir_entity *method;
				printf("\t\t\t\t\tup to %u methods to enable\n", cpset_size(methods));
				while ((method = cpset_iterator_next(&iterator)) != NULL) {
					add_to_dyncalls(method, method, env); // search recursively upwards if overwritten entities had been called and add it to their target sets
					//cpset_remove_iterator(methods, &iterator);
				}
			}
			// remove the map entry and free the set
			cpmap_remove(env->disabled_targets, klass);
			cpset_destroy(methods);
			free(methods);
		}
	}
}

static void memorize_disabled_method(ir_type *klass, ir_entity *entity, analyzer_env *env) {
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

static void handle_external_method(ir_entity *method, analyzer_env *env) {
	assert(is_method_entity(method));
	assert(env);

	assert(!is_inherited_copy(method));
	if (cpset_find(env->external_methods, method) == NULL) { // check if not already handled this method
		printf("\t\thandling external method %s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));

		// something to do?

		cpset_insert(env->external_methods, method);
	}
}

static void add_to_workqueue(ir_entity *method, analyzer_env *env) {
	assert(is_method_entity(method));
	assert(env);

	assert(!is_inherited_copy(method));
	ir_graph *graph = get_entity_irg(method);
	if (graph) {
		if (cpset_find(env->done_set, graph) == NULL && cpset_find(env->in_queue, graph) == NULL) { // only enqueue if not already done or enqueued
			printf("\t\tadding %s.%s to workqueue\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			pdeq_putr(env->workqueue, graph);
			cpset_insert(env->in_queue, graph);
		}
	} else {
		// treat methods without graph as external methods
		// since we can't analyze it, mark all potentially returned object types as in use (completely down the class hierarchy!)
		printf("\t\tfound no graph, probably external\n");
		handle_external_method(method, env);
	}
}

static void take_entity(ir_entity *entity, cpset_t *result_set, analyzer_env *env) {
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	if (cpset_find(result_set, entity) == NULL) { // take each entity only once (the sets won't mind but the workqueue)
		printf("\t\ttaking entity %s.%s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity));

		// add to live methods
		cpset_insert(env->live_methods, entity);

		// add to result set
		cpset_insert(result_set, entity);

		// add to workqueue
		add_to_workqueue(entity, env);
	}
}

// collect method entities from downwards in the class hierarchy
// /!\ should normally be called with the owner class of the entity e.g. to start at the static looked up entity
// it walks down the classes to have the entities with the classes even when the method is inherited
static void collect_methods(ir_type *klass, ir_entity *entity, cpset_t *result_set, analyzer_env *env) {
	assert(is_Class_type(klass));
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	printf("\t\twalking class %s\n", get_class_name(klass));
	ir_entity *current_entity = entity;
	ir_entity *overwriting_entity = get_class_member_by_name(klass, get_entity_ident(current_entity)); //?? Is there a more efficient way? // /!\ note: only works because whole signature is already encoded in entity name!
	if (overwriting_entity != NULL && overwriting_entity != current_entity) { // if has overwriting entity
		assert(klass == get_entity_owner(overwriting_entity));
		printf("\t\t\t%s.%s overwrites %s.%s\n", get_class_name(get_entity_owner(overwriting_entity)), get_entity_name(overwriting_entity), get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity));

		current_entity = overwriting_entity;
	}
	//else // inherited

	while (is_inherited_copy(current_entity)) { // use copied entities of inherited methods to find implementations (especially in the case when interface method is implemented by a superclass)
		ir_entity *impl_entity = ddispatch_get_bound_entity(current_entity);
		assert(impl_entity);
		assert(!oo_get_class_is_interface(get_entity_owner(impl_entity)));
		assert(!oo_get_method_is_abstract(impl_entity));
		printf("\t\t\tfound copied method entity %s.%s -> %s.%s\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity), get_class_name(get_entity_owner(impl_entity)), get_entity_name(impl_entity));

		current_entity = impl_entity;
	}

	if (!oo_get_method_is_abstract(current_entity)) { // ignore abstract methods
		if (cpset_find(env->live_classes, klass) != NULL || oo_get_class_is_extern(klass) || JUST_CHA) { // if class is considered in use
			take_entity(current_entity, result_set, env);
		} else {
			printf("\t\t\tclass not in use, memorizing %s.%s\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity));
			memorize_disabled_method(klass, current_entity, env); // remember entity with this class for patching if this class will become used
		}
	} else
		printf("\t\t\t%s.%s is abstract\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity));

	for (size_t i=0; i<get_class_n_subtypes(klass); i++) {
		ir_type *subclass = get_class_subtype(klass, i);

		collect_methods(subclass, current_entity, result_set, env);
	}
}


static void walk_callgraph_and_analyze(ir_node *node, void *environment) {
	assert(environment);
	analyzer_env *env = (analyzer_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Store: {
		// look for store of vptrs to detect object creation
		ir_node *dest =  get_irn_n(node, 1);
		ir_node *src =  get_irn_n(node, 2);
		printf("\tstore: %s\n", gdb_node_helper(dest));
		if (is_Sel(dest)) {
			ir_entity *entity = get_Sel_entity(dest);
			printf("\t\t%s\n", get_entity_name(entity));
			if (strncmp(get_entity_name(entity), "@vptr", 5) == 0) {
				printf("\t\t\t%s\n", gdb_node_helper(src));

				assert(is_Add(src)); // src is usually an Add node in bytecode2firm //TODO write code that correctly parses what liboo function ddispatch_prepare_new_instance creates according to the configured vtable layout
				ir_node *lhs = get_irn_n(src, 0);
				assert(is_Address(lhs));
				ir_entity *vtable_entity = get_Address_entity(lhs);
				printf("\t\t\t\tLHS: %s -> %s -> owner: %s\n", gdb_node_helper(lhs), get_entity_name(vtable_entity), get_class_name(get_entity_owner(vtable_entity)));
				ir_type *klass = cpmap_find(env->vtable2class, vtable_entity);
				assert(is_Class_type(klass));
				printf("\t\t\t\t\t-> vtable2class: %s\n", get_class_name(klass));

				add_new_live_class(klass, env);

				printf("\t\t\t\tRHS: %s\n", gdb_node_helper(get_irn_n(src, 1))); // rhs
			}
		}
		break;
	}
	case iro_Call: {
		ir_node *callee = get_irn_n(node, 1);
		if (is_Address(callee)) {
			// handle static call
			ir_entity *entity = get_Address_entity(callee);
			printf("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
			//printf("\t\t\toverwrites: %u\n", get_entity_n_overwrites(entity));
			//printf("\t\t\toverwrittenby: %u\n", get_entity_n_overwrittenby(entity));
			//printf("\t\t\tis abstract method: %u\n", oo_get_method_is_abstract(entity));
			//printf("\t\t\tis interface method: %u\n", oo_get_class_is_interface(get_entity_owner(entity)));
			//printf("\t\t\tis owner extern: %u\n", oo_get_class_is_extern(get_entity_owner(entity)));

			// add to live methods
			cpset_insert(env->live_methods, entity);

			add_to_workqueue(entity, env);


			// hack to detect object creation or calls (like class initialization) that are hidden in frontend-specific nodes
			ir_graph *graph = get_entity_irg(entity);
			if (!graph) {
				// ask frontend if there are objects created here (e.g. needed to detect GCJ object creation)
				ir_type *klass = detect_creation(node); //TODO support for more than one
				if (klass) {
					printf("\t\texternal method creates class %s\n", get_class_name(klass));
					add_new_live_class(klass, env);
				}

				// ask frontend if there are additional methods called here (e.g. needed to detect class initialization)
				ir_entity *called_method = detect_call(node); //TODO support for more than one
				if (called_method) {
					assert(is_method_entity(called_method));
					//assert(get_entity_irg(called_method)); // can be external
					printf("\t\texternal method calls %s.%s (%s)\n", get_class_name(get_entity_owner(called_method)), get_entity_name(called_method), get_entity_ld_name(called_method));
					cpset_insert(env->live_methods, called_method);
					add_to_workqueue(called_method, env);
				}
			}


		} else if (is_Proj(callee)) {
			callee = get_Proj_pred(callee);
			if (is_MethodSel(callee)) {
				// handle dynamic call
				ir_entity *entity = get_MethodSel_entity(callee);
				printf("\tdynamic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
				//printf("\t\t\toverwrites: %u\n", get_entity_n_overwrites(entity));
				//printf("\t\t\toverwrittenby: %u\n", get_entity_n_overwrittenby(entity));
				//printf("\t\t\tis abstract method: %u\n", oo_get_method_is_abstract(entity));
				//printf("\t\t\tis interface method: %u\n", oo_get_class_is_interface(get_entity_owner(entity)));
				//printf("\t\t\tis owner extern: %u\n", oo_get_class_is_extern(get_entity_owner(entity)));

				if (cpmap_find(env->dyncall_targets, entity) == NULL) { // if not already done
					// calculate set of method entities that this call could potentially call

					// static lookup upwards in the class hierarchy (gets just one method entity)
					// The entity from the Sel node is already what the result of a static lookup would be.

					// collect all potentially called method entities from downwards the class hierarchy
					cpset_t *result_set = new_cpset(hash_ptr, ptr_equals);
					ir_type *owner = get_entity_owner(entity);
					collect_methods(owner, entity, result_set, env);

					// note: we can't check here for a nonempty result set because classes could be nonlive at this point but become live later depending on the order in which methods are analyzed

					cpmap_set(env->dyncall_targets, entity, result_set);
				}
			} else
				assert(false); // neither Address nor Proj to MethodSel as callee shouldn't happen!?
		} else
			assert(false); // neither Address nor Proj to MethodSel as callee shouldn't happen!?
		break;
	}
	default:
		// skip other node types
		break;
	}
}


typedef struct class_collector_env {
	cpmap_t *vtable2class;
} class_collector_env;

static void walk_classes_and_collect(ir_type *klass, void* environment) {
	assert(environment);
	class_collector_env *env = (class_collector_env*)environment;

	cpmap_t *vtable2class = env->vtable2class;
	ir_entity *vtable_entity = oo_get_class_vtable_entity(klass);
	if (vtable_entity) {
		//printf(" %s -> %s\n", get_entity_name(vtable_entity), gdb_node_helper(klass));
		cpmap_set(vtable2class, vtable_entity, klass);
	}
	else {
		//printf(" %s has no vtable\n", gdb_node_helper(klass));
		//printf("\tis interface: %u\n", oo_get_class_is_interface(klass));
		//printf("\tis extern: %u\n", oo_get_class_is_extern(klass));
	}

/*	size_t n = get_class_n_supertypes(klass);
	printf("\t%u ", n);
	for (size_t i=0; i<n; i++) {
		ir_type *super = get_class_supertype(klass, i);
		printf("%s", get_class_name(super));
		if (i<n-1) printf(", ");
	}
	printf("\n");
*/
}


/** run Rapid Type Analysis
 * It runs over a reduced callgraph and detects which classes and methods are actually used and computes reduced sets of potentially called targets for each dynamically linked call.
 * @note RTA must know of _all_ definitely executed code parts (main, static sections, global contructors or all nonprivate functions if it's a library)! It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!!
 * @note RTA also won't work with programs that dynamically load classes at run-time! It can lead to defective programs!!
 * @note RTA might also produce incorrect programs if there is some Java Reflections shenanigans in the code (internal or external), especially using java.lang.Class.newInstance()
 * @note Give classes that are instantiated in native methods of a nonexternal standard library or runtime as initial_live_classes, give methods called in these native methods as additional entry points. If something is missing, RTA could produce incorrect programs! This also means that native methods in the program that do arbitrary things are not supported.
 * @param entry_points NULL-terminated array of method entities, give all entry points to program code, may _not_ be NULL and must contain at least one method entity, also all entry points should have a graph
 * @param initial_live_classes NULL-terminated array of classes that should always be considered live, may be NULL
 * @param live_classes give pointer to empty uninitialized set for receiving results, This is where all live classes are put (as ir_type*).
 * @param live_methods give pointer to empty uninitialized set for receiving results, This is where all live methods are put (as ir_entity*).
 * @param dyncall_targets give pointer to empty uninitialized map for receiving results, This is where call entities are mapped to their actually used potential call targets (ir_entity* -> {ir_entity*}). It's used to optimize dynamically linked calls if possible. (see also function rta_optimize_dyncalls)
 */
static void rta_run(ir_entity **entry_points, ir_type **initial_live_classes, cpset_t *live_classes, cpset_t *live_methods, cpmap_t *dyncall_targets) {
	assert(entry_points);
	assert(live_classes);
	assert(live_methods);
	assert(dyncall_targets);

	cpset_init(live_classes, hash_ptr, ptr_equals);
	cpset_init(live_methods, hash_ptr, ptr_equals);
	cpmap_init(dyncall_targets, hash_ptr, ptr_equals);

	cpmap_t vtable2class;
	cpmap_init(&vtable2class, hash_ptr, ptr_equals);
	{ // walk all classes to fill the vtable2class map
		class_collector_env env = { .vtable2class = &vtable2class };
		class_walk_super2sub(walk_classes_and_collect, NULL, &env);
	}
	printf("number of classes with vtables: %u\n", cpmap_size(&vtable2class));

	cpmap_t disabled_targets;
	cpmap_init(&disabled_targets, hash_ptr, ptr_equals);

	cpset_t external_methods;
	cpset_init(&external_methods, hash_ptr, ptr_equals);

	pdeq *workqueue = new_pdeq();

	cpset_t in_queue;
	cpset_init(&in_queue, hash_ptr, ptr_equals);

	cpset_t done_set;
	cpset_init(&done_set, hash_ptr, ptr_equals);

	analyzer_env env = {
		.workqueue = workqueue,
		.in_queue = &in_queue,
		.done_set = &done_set,
		.vtable2class = &vtable2class,
		.live_classes = live_classes,
		.live_methods = live_methods,
		.dyncall_targets = dyncall_targets,
		.disabled_targets = &disabled_targets,
		.external_methods = &external_methods
	};

	{ // add all given entry points to live methods and to workqueue
		ir_entity *entry;
		size_t i = 0;
		for (; (entry = entry_points[i]) != NULL; i++) {
			assert(is_method_entity(entry));
			cpset_insert(live_methods, entry);

			// add to workqueue
			ir_graph *graph = get_entity_irg(entry);
			assert(is_ir_graph(graph)); // don't give methods without a graph as entry points for the analysis !? TODO
			// note: omiting to check if already in queue assuming no duplicates in given entry points
			pdeq_putr(workqueue, graph);
			cpset_insert(&in_queue, graph);
		}
		assert(i > 0 && "give at least one entry point");
	}

	// add all given initial live classes to live classes
	if (initial_live_classes != NULL) {
		ir_type *entry;
		for (size_t i=0; (entry = initial_live_classes[i]) != NULL; i++) {
			assert(is_Class_type(entry));
			cpset_insert(live_classes, entry);
		}
	}

	while (!pdeq_empty(workqueue)) {
		ir_graph *g = pdeq_getl(workqueue);
		assert(is_ir_graph(g));
		cpset_remove(&in_queue, g);

		if (cpset_find(&done_set, g) != NULL) continue;

		printf("\n== %s.%s\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)));

		cpset_insert(&done_set, g); // mark as done _before_ walking to not add it again in case of recursive calls
		irg_walk_graph(g, NULL, walk_callgraph_and_analyze, &env);
	}
	assert(cpset_size(&in_queue) == 0);


	printf("\n\n==== Results ==============================================\n");
	{
		printf("\nlive classes:\n");
		cpset_iterator_t iterator;
		cpset_iterator_init(&iterator, live_classes);
		ir_type *klass;
		while ((klass = cpset_iterator_next(&iterator)) != NULL) {
			printf("\t%s\n", get_class_name(klass));
		}
	}
	{
		printf("\nlive methods:\n");
		cpset_iterator_t iterator;
		cpset_iterator_init(&iterator, live_methods);
		ir_entity *method;
		while ((method = cpset_iterator_next(&iterator)) != NULL) {
			//printf("\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
			printf("\t%s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
		}
	}
	{
		printf("\ndyncall target sets:\n");
		//printf("size %u\n", cpmap_size(dyncall_targets));
		cpmap_iterator_t iterator;
		cpmap_iterator_init(&iterator, dyncall_targets);
		cpmap_entry_t *entry;
		while ((entry = cpmap_iterator_next(&iterator))->key != NULL || entry->data != NULL) {
			const ir_entity *call_entity = entry->key;
			assert(call_entity);
			printf("\t%s.%s %s\n", get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity), (oo_get_class_is_extern(get_entity_owner(call_entity))) ? "external" : "");

			cpset_t *targets = entry->data;
			assert(targets);
			cpset_iterator_t it;
			cpset_iterator_init(&it, targets);
			ir_entity *method;
			while ((method = cpset_iterator_next(&it)) != NULL) {
				//printf("\t\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
				printf("\t\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), (oo_get_class_is_extern(get_entity_owner(call_entity))) ? "external" : "");
			}
		}
	}
	printf("\n=============================================================\n");

	// free data structures
	del_pdeq(workqueue);
	cpset_destroy(&in_queue);
	cpset_destroy(&done_set);
	cpmap_destroy(&vtable2class);

	{ // delete the sets in map disabled_targets
		cpmap_iterator_t it;
		cpmap_iterator_init(&it, &disabled_targets);
		cpmap_entry_t *entry;
		while ((entry = cpmap_iterator_next(&it))->key != NULL || entry->data != NULL) {
			cpset_t* set = entry->data;
			cpmap_remove_iterator(&disabled_targets, &it);
			assert(set);
			cpset_destroy(set);
			free(set);
		}
	}
	cpmap_destroy(&disabled_targets);
	cpset_destroy(&external_methods);

	// /!\ live_classes, live_methods and dyncall_targets are given from outside and return the results, but the sets in map dyncall_targets are allocated in the process and have to be deleted later

}


/** frees memory allocated for the results returned by function run_rta
 * @note does not free the memory of the sets and maps themselves, just their content allocated during RTA
 * @param live_classes as returned by rta_run
 * @param live_methods as returned by rta_run
 * @param dyncall_targets as returned by rta_run
 */
static void rta_dispose_results(cpset_t *live_classes, cpset_t *live_methods, cpmap_t *dyncall_targets) {
	assert(live_classes);
	assert(live_methods);
	assert(dyncall_targets);

	cpset_destroy(live_classes);
	cpset_destroy(live_methods);

	// delete the sets in map disabled_targets
	cpmap_iterator_t it;
	cpmap_iterator_init(&it, dyncall_targets);
	cpmap_entry_t *entry;
	while ((entry = cpmap_iterator_next(&it))->key != NULL || entry->data != NULL) {
		cpset_t* set = entry->data;
		cpmap_remove_iterator(dyncall_targets, &it);
		assert(set);
		cpset_destroy(set);
		free(set);
	}
	cpmap_destroy(dyncall_targets);
}


typedef struct optimizer_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpset_t *in_queue; // set to avoid duplicates in the workqueue
	cpset_t *done_set; // set to mark graphs that were already analyzed
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically linked call (Map: call entity -> Set: method entities)
} optimizer_env;

static void optimizer_add_to_workqueue(ir_entity *method, optimizer_env *env) {
	assert(is_method_entity(method));
	assert(env);

	ir_graph *graph = get_entity_irg(method);
	if (graph) {
		if (cpset_find(env->done_set, graph) == NULL && cpset_find(env->in_queue, graph) == NULL) { // only enqueue if not already done or enqueued
			printf("\t\tadding %s.%s to workqueue\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			pdeq_putr(env->workqueue, graph);
			cpset_insert(env->in_queue, graph);
		}
	}
}

static void walk_callgraph_and_devirtualize(ir_node *node, void* environment) {
	assert(environment);
	optimizer_env *env = (optimizer_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Call: {
		ir_node *call = node;
		ir_node *callee = get_irn_n(call, 1);
		if (is_Address(callee)) {
			// handle static call
			ir_entity *entity = get_Address_entity(callee);
			printf("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

			optimizer_add_to_workqueue(entity, env);


			// hack to detect calls (like class initialization) that are hidden in frontend-specific nodes
			ir_graph *graph = get_entity_irg(entity);
			if (!graph) {
				ir_entity *called_method = detect_call(call);
				if (called_method) {
					assert(is_method_entity(called_method));
					//assert(get_entity_irg(called_method)); // can be external
					printf("\t\texternal method calls %s.%s (%s)\n", get_class_name(get_entity_owner(called_method)), get_entity_name(called_method), get_entity_ld_name(called_method));
					optimizer_add_to_workqueue(called_method, env);
				}
			}


		} else if (is_Proj(callee)) {
			ir_node *pred = get_Proj_pred(callee);
			if (is_MethodSel(pred)) {
				ir_node *methodsel = pred;
				// handle dynamic call
				ir_entity *entity = get_MethodSel_entity(methodsel);
				ir_type *owner = get_entity_owner(entity);
				printf("\tdynamic call: %s.%s %s\n", get_class_name(owner), get_entity_name(entity), gdb_node_helper(entity));

				cpset_t *targets = cpmap_find(env->dyncall_targets, entity);
				assert(targets);
				assert(cpset_size(targets) > 0 || (oo_get_class_is_extern(owner) && !oo_get_class_is_final(owner) && !oo_get_method_is_final(entity))); // if we found no call targets then something went wrong (except it was a call to a nonfinal method of a nonfinal external class because then there can be unknown external subclasses that overwrite the method)
				if (cpset_size(targets) == 1 && (!oo_get_class_is_extern(owner) || oo_get_class_is_final(owner) || oo_get_method_is_final(entity))) {
					// devirtualize call
					cpset_iterator_t it;
					cpset_iterator_init(&it, targets);
					ir_entity *target = cpset_iterator_next(&it);
					assert(cpset_iterator_next(&it) == NULL);

					printf("\t\tdevirtualizing call %s.%s -> %s.%s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), get_class_name(get_entity_owner(target)), get_entity_name(target));
					// set an Address node as callee
					ir_graph *graph = get_irn_irg(callee);
					ir_node *address = new_r_Address(graph, target);
					set_irn_n(call, 1, address);
					// disconnect MethodSel node from Mem edge
					ir_node *mem = get_irn_n(methodsel, 0);
					set_irn_n(callee, 0, get_irg_no_mem(graph));
					ir_node *projm = get_irn_n(call, 0);
					ir_node *pred;
					while ((pred = get_irn_n(projm, 0)) != methodsel) // find correct ProjM node
						projm = pred;
					assert(is_Proj(projm));
					exchange(projm, mem);
				}

				// add to workqueue
				cpset_iterator_t it;
				cpset_iterator_init(&it, targets);
				ir_entity *target;
				while ((target = cpset_iterator_next(&it)) != NULL) {
					optimizer_add_to_workqueue(target, env);
				}
			} else
				assert(false); // neither Address nor Proj to MethodSel as callee shouldn't happen!?
		} else
			assert(false); // neither Address nor Proj to MethodSel as callee shouldn't happen!?
		break;
	}
	default:
		// skip other node types
		break;
	}

}

/** devirtualizes dyncalls if their target set contains only one entry
 * @param entry_points same as used with rta_run
 * @param dyncall_targets the result map returned from rta_run
 */
static void rta_devirtualize_calls(ir_entity **entry_points, cpmap_t *dyncall_targets) {
	assert(dyncall_targets);

	pdeq *workqueue = new_pdeq();

	cpset_t in_queue;
	cpset_init(&in_queue, hash_ptr, ptr_equals);

	cpset_t done_set;
	cpset_init(&done_set, hash_ptr, ptr_equals);

	optimizer_env env = {
		.workqueue = workqueue,
		.in_queue = &in_queue,
		.done_set = &done_set,
		.dyncall_targets = dyncall_targets
	};

	{ // add all given entry points to workqueue
		ir_entity *entry;
		for (size_t i=0; (entry = entry_points[i]) != NULL; i++) {
			assert(is_method_entity(entry));
			ir_graph *graph = get_entity_irg(entry);
			assert(is_ir_graph(graph)); // don't give methods without a graph as entry points for the analysis !? TODO
			// note: omiting to check if already in queue assuming no duplicates in given entry points
			pdeq_putr(workqueue, graph);
			cpset_insert(&in_queue, graph);
		}
	}

	while (!pdeq_empty(workqueue)) {
		ir_graph *g = pdeq_getl(workqueue);
		assert(is_ir_graph(g));
		cpset_remove(&in_queue, g);

		if (cpset_find(&done_set, g) != NULL) continue;

		printf("\n== %s.%s\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)));

		cpset_insert(&done_set, g); // mark as done _before_ walking to not add it again in case of recursive calls
		irg_walk_graph(g, NULL, walk_callgraph_and_devirtualize, &env);
	}
	assert(cpset_size(&in_queue) == 0);

	// free data structures
	del_pdeq(workqueue);
	cpset_destroy(&in_queue);
	cpset_destroy(&done_set);
}


void rta_optimization(ir_entity **entry_points, ir_type **initial_live_classes) {
	assert(entry_points);

	cpset_t live_classes;
	cpset_t live_methods;
	cpmap_t dyncall_targets;

	rta_run(entry_points, initial_live_classes, &live_classes, &live_methods, &dyncall_targets);
	rta_devirtualize_calls(entry_points, &dyncall_targets);
	//rta_discard(&live_classes, &live_methods); ??
	rta_dispose_results(&live_classes, &live_methods, &dyncall_targets);
}
