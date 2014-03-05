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

#include "adt/cpmap.h"
#include "adt/cpset.h"
#include "adt/pdeq.h"
#include "adt/hashptr.h"


// override option just for early development to keep going without information about used types
#define JUST_CHA 0


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

typedef struct analyzer_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpmap_t *vtable2class; // map for finding the class to a given vtable entity
	cpset_t *used_classes; // used classes found by examining object creation or coming in from external functions
	cpset_t *used_methods; // used method entities
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically linked call (Map: call entity -> Set: method entities)
	cpmap_t *disabled_targets; // map that stores the set of disabled potential call targets of dynamic calls for every class) (Map: class -> Set: method entities)
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

static void add_new_used_class(ir_type *klass, analyzer_env *env) {
	assert(is_Class_type(klass));
	assert(env);

	if (cpset_find(env->used_classes, klass) == NULL // if it had not already been added
	    && !oo_get_class_is_abstract(klass)) { // if not abstract
		// add to used classes
		cpset_insert(env->used_classes, klass);
		printf("\t\t\t\t\tadded new used class %s\n", get_class_name(klass));

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

static void add_all_subclasses(ir_type *klass, analyzer_env *env) {
	assert(is_Class_type(klass));
	assert(env);

	// add itself
	add_new_used_class(klass, env);

	// add recursively all subclasses
	for (size_t i=0; i<get_class_n_subtypes(klass); i++) {
		ir_type *subclass = get_class_subtype(klass, i);
		add_all_subclasses(subclass, env);
	}
}

static void handle_external_method(ir_entity *method, analyzer_env *env) {
	assert(is_method_entity(method));
	assert(env);

	assert(!oo_get_method_is_inherited(method));
	printf("\t\thandling external method %s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
	//TODO what if we already had handled this external method?
	ir_type *methodtype = get_entity_type(method);
	for (size_t i=0; i<get_method_n_ress(methodtype); i++) {
		ir_type *type = get_method_res_type(methodtype, i);
		printf("\t\t\tresult type %s\n", gdb_node_helper(type));
		if (is_Pointer_type(type)) {
			while (is_Pointer_type(type))
				type = get_pointer_points_to_type(type);
			if (is_Class_type(type)) {
				// add class and all its subclasses to used types
				add_all_subclasses(type, env);
			}
		} else
			assert(!is_Class_type(type)); // modern languages shouldn't have class types without pointer
	}
}

static void add_to_workqueue(ir_entity *method, analyzer_env *env) {
	assert(is_method_entity(method));
	assert(env);

	ir_graph *graph = get_entity_irg(method);
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

static void take_entity(ir_entity *entity, cpset_t *result_set, analyzer_env *env) {
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	if (cpset_find(result_set, entity) == NULL) { // take each entity only once (the sets won't mind but the workqueue)
		printf("\t\ttaking entity %s.%s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity));

		// add to used methods
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

	while (oo_get_method_is_inherited(current_entity)) { // use copied entities of inherited methods to find implementations (especially in the case when interface method is implemented by a superclass)
		ir_entity *impl_entity = oo_get_entity_overwritten_superclass_entity(current_entity);
		//assert(!oo_get_method_is_inherited(impl_entity));
		assert(!oo_get_class_is_interface(get_entity_owner(impl_entity)));
		printf("\t\t\tfound copied method entity %s.%s -> %s.%s\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity), get_class_name(get_entity_owner(impl_entity)), get_entity_name(impl_entity));

		current_entity = impl_entity;
	}

	if (!oo_get_method_is_abstract(current_entity)) { // ignore abstract methods
		if (cpset_find(env->used_classes, klass) != NULL || JUST_CHA) { // if class is actually in use
			take_entity(current_entity, result_set, env);
		} else {
			memorize_disabled_method(klass, current_entity, env); // remember entity with this class for patching if this class will become used
		}
	}

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

				add_new_used_class(klass, env);

				printf("\t\t\t\tRHS: %s\n", gdb_node_helper(get_irn_n(src, 1))); // rhs
			}
		}
		break;
	}
	case iro_Load: {
		// look for load of non-private fields of external classes to detect used objects whose creations we can't analyze elseway
		// note: This is necessary because we can't analyze the standard library.
		// note: We can't check for "public" so we have to do it for _all_ fields (which is very suboptimal).
		ir_node *src =  get_irn_n(node, 1);
		printf("\tload: %s\n", gdb_node_helper(src));
		if (is_Address(src)) { // static field
			ir_entity *entity = get_Address_entity(src);
			printf("\t\t%s\n", get_entity_name(entity));
			ir_type *owner = get_entity_owner(entity);
			printf("\t\t\towner: %s\n", get_class_name(owner));
			ir_type *type = get_entity_type(entity);
			while (is_Pointer_type(type))
				type = get_pointer_points_to_type(type);
			printf("\t\t\t(points to) type: %s\n", gdb_node_helper(type));

			// note: problem is we can't say where the static field came from (extern class or not)
			if (is_Class_type(type)) {
				printf("\t\t\tfield entity is extern: %u\n", get_entity_visibility(entity) == ir_visibility_external);
				add_all_subclasses(type, env);
			}
		} else if (is_Sel(src)) { // nonstatic field
			ir_entity *entity = get_Sel_entity(src);
			ir_type *owner = get_entity_owner(entity);
			//printf("\t\t\towner: %s\n", gdb_node_helper(owner));
			if (is_Class_type(owner)) {
				printf("\t\t%s\n", get_entity_name(entity));
				printf("\t\t\towner: %s\n", get_class_name(owner));
				ir_type *type = get_entity_type(entity);
				while (is_Pointer_type(type))
					type = get_pointer_points_to_type(type);
				printf("\t\t\t(points to) type: %s\n", gdb_node_helper(type));

				printf("\t\t\tfield entity is extern: %u\n", get_entity_visibility(entity) == ir_visibility_external);
				if (oo_get_class_is_extern(owner)) { // add to used classes only if its an field of an external class
					if (is_Class_type(type)) {
						add_all_subclasses(type, env);
					}
				}
			} //else // array access or other
		} else
			assert(false); // shouldn't happen!??? Are there more possibilities than Address and Sel?
		break;
	}
	case iro_Call: {
		ir_node *fp = get_irn_n(node, 1);
		if (is_Address(fp)) {
			// handle static call
			ir_entity *entity = get_Address_entity(fp);
			printf("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
			//printf("\t\t\toverwrites: %u\n", get_entity_n_overwrites(entity));
			//printf("\t\t\toverwrittenby: %u\n", get_entity_n_overwrittenby(entity));
			//printf("\t\t\tis abstract method: %u\n", oo_get_method_is_abstract(entity));
			//printf("\t\t\tis interface method: %u\n", oo_get_class_is_interface(get_entity_owner(entity)));
			//printf("\t\t\tis owner extern: %u\n", oo_get_class_is_extern(get_entity_owner(entity)));

			// add to used methods
			cpset_insert(env->used_methods, entity);

			add_to_workqueue(entity, env);

		} else if (is_Sel(fp)) {
			// handle dynamic call
			ir_entity *entity = get_Sel_entity(fp);
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

				assert(cpset_size(result_set) > 0);

				cpmap_set(env->dyncall_targets, entity, result_set); //??
			}
		} else
			assert(false); // neither Address nor Sel as callee shouldn't happen!?
		break;
	}
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
 * @note RTA needs to know about _all_ classes. (The class hierarchy graph(s) must be complete.) This means it is necessary to load _all_ inner dependencies of external library classes! Otherwise the results can be incorrect and can lead to defective programs!!
 * @param entry_points all (public) entry points to program code (as ir_entity*)
 * @param used_classes give pointer to empty uninitialized set for receiving results, This is a set where all used classes are put (as ir_type*).
 * @param used_methods give pointer to empty uninitialized set for receiving results, This is a set where all used methods are put (as ir_entity*).
 * @param dyncall_targets give pointer to empty uninitialized map for receiving results, This is a map where call entities are mapped to their actually used potential call targets (ir_entity* -> {ir_entity*}). It's used to optimize dynamically linked calls if possible. (see also function rta_optimize_dyncalls)
 */
static void rta_run(cpset_t *entry_points, cpset_t *used_classes, cpset_t *used_methods, cpmap_t *dyncall_targets) {
	assert(entry_points);
	assert(used_classes);
	assert(used_methods);
	assert(dyncall_targets);

	assert(cpset_size(entry_points) != 0);

	cpset_init(used_classes, hash_ptr, ptr_equals);
	cpset_init(used_methods, hash_ptr, ptr_equals);
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

	pdeq *workqueue = new_pdeq();

	analyzer_env env = {
		.workqueue = workqueue,
		.vtable2class = &vtable2class,
		.used_classes = used_classes,
		.used_methods = used_methods,
		.dyncall_targets = dyncall_targets,
		.disabled_targets = &disabled_targets
	};

	{ // add all given entry points to workqueue and check their parameters for class types
		cpset_iterator_t it;
		cpset_iterator_init(&it, entry_points);
		ir_entity *entry;
		while ((entry = cpset_iterator_next(&it)) != NULL) {
			assert(is_method_entity(entry));

			// add to used methods
			cpset_insert(used_methods, entry);

			// add to workqueue
			ir_graph *graph = get_entity_irg(entry);
			assert(is_ir_graph(graph)); // don't give methods without a graph as entry points for the analysis !? TODO
			pdeq_putr(workqueue, graph);

			// check parameters for incoming class types
			printf("entry point: %s.%s %s\n", get_class_name(get_entity_owner(entry)), get_entity_name(entry), gdb_node_helper(entry));
			ir_type *methodtype = get_entity_type(entry);
			assert(is_Method_type(methodtype));
			for (size_t i=0; i<get_method_n_params(methodtype); i++) {
				ir_type *type = get_method_param_type(methodtype, i);
				printf("\tparameter type %s\n", gdb_node_helper(type));
				if (is_Pointer_type(type)) {
					while (is_Pointer_type(type))
						type = get_pointer_points_to_type(type);
					if (is_Class_type(type)) {
						printf("\t\tfound class type: %s\n", get_class_name(type));
						add_all_subclasses(type, &env);
					}
				} else
					assert(!is_Class_type(type)); // class type without pointer shouldn't happen
			}

		}
	}

	cpset_t done_set;
	cpset_init(&done_set, hash_ptr, ptr_equals);
	while (!pdeq_empty(workqueue)) {
		ir_graph *g = pdeq_getl(workqueue);
		assert(is_ir_graph(g));

		if (cpset_find(&done_set, g) != NULL) continue;

		printf("\n== %s.%s\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)));

		cpset_insert(&done_set, g); // mark as done _before_ walking because of possible recursive calls !??? -> not necessary but not wrong!? TODO
		irg_walk_graph(g, NULL, walk_callgraph_and_analyze, &env);
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
		printf("\ndyncall target sets:\n");
		//printf("size %u\n", cpmap_size(dyncall_targets));
		cpmap_iterator_t iterator;
		cpmap_iterator_init(&iterator, dyncall_targets);
		cpmap_entry_t *entry;
		while ((entry = cpmap_iterator_next(&iterator))->key != NULL || entry->data != NULL) {
			const ir_entity *call_entity = entry->key;
			assert(call_entity);
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
	printf("\n=============================================================\n");

	// free data structures
	del_pdeq(workqueue);
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

	cpset_destroy(&done_set);

	// /!\ used_classes, used_methods and dyncall_targets are given from outside and return the results, but the sets in map dyncall_targets are allocated in the process and have to be deleted later

}


/** frees memory allocated for the results returned by function run_rta
 * @note does not free the memory of the sets and maps themselves, just their content allocated during RTA
 * @param used_classes as returned by rta_run
 * @param used_methods as returned by rta_run
 * @param dyncall_targets as returned by rta_run
 */
static void rta_dispose_results(cpset_t *used_classes, cpset_t *used_methods, cpmap_t *dyncall_targets) {
	assert(used_classes);
	assert(used_methods);
	assert(dyncall_targets);

	cpset_destroy(used_classes);
	cpset_destroy(used_methods);

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
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically linked call (Map: call entity -> Set: method entities)
} optimizer_env;

static void optimizer_add_to_workqueue(ir_entity *method, optimizer_env *env) {
	assert(is_method_entity(method));
	assert(env);

	ir_graph *graph = get_entity_irg(method);
	if (graph) {
		printf("\t\tadding %s.%s to workqueue\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
		pdeq_putr(env->workqueue, graph);
	}
}

static void walk_callgraph_and_devirtualize(ir_node *node, void* environment) {
	assert(environment);
	optimizer_env *env = (optimizer_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Call: {
		ir_node *fp = get_irn_n(node, 1);
		if (is_Address(fp)) {
			// handle static call
			ir_entity *entity = get_Address_entity(fp);
			printf("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

			optimizer_add_to_workqueue(entity, env);

		} else if (is_Sel(fp)) {
			// handle dynamic call
			ir_entity *entity = get_Sel_entity(fp);
			printf("\tdynamic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

			cpset_t *targets = cpmap_find(env->dyncall_targets, entity);
			assert(targets);
			assert(cpset_size(targets) > 0);
			if (cpset_size(targets) == 1) {
				// devirtualize call
				cpset_iterator_t it;
				cpset_iterator_init(&it, targets);
				ir_entity *target = cpset_iterator_next(&it);
				assert(cpset_iterator_next(&it) == NULL);

				printf("\t\tdevirtualizing call %s.%s -> %s.%s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), get_class_name(get_entity_owner(target)), get_entity_name(target));
				ir_node *address = new_Address(target);
				set_irn_n(node, 1, address);
			}

			// add to workqueue
			cpset_iterator_t it;
			cpset_iterator_init(&it, targets);
			ir_entity *target;
			while ((target = cpset_iterator_next(&it)) != NULL) {
				optimizer_add_to_workqueue(target, env);
			}
		}
	}
	}

}

/** devirtualizes dyncalls if their target set contains only one entry
 * @param entry_points same as used with rta_run
 * @param dyncall_targets the result map returned from rta_run
 */
static void rta_devirtualize_calls(cpset_t *entry_points, cpmap_t *dyncall_targets) {
	assert(dyncall_targets);

	pdeq *workqueue = new_pdeq();

	optimizer_env env = {
		.workqueue = workqueue,
		.dyncall_targets = dyncall_targets,
	};

	{ // add all given entry points to workqueue
		cpset_iterator_t it;
		cpset_iterator_init(&it, entry_points);
		ir_entity *entry;
		while ((entry = cpset_iterator_next(&it)) != NULL) {
			assert(is_method_entity(entry));
			ir_graph *graph = get_entity_irg(entry);
			assert(is_ir_graph(graph)); // don't give methods without a graph as entry points for the analysis !? TODO
			pdeq_putr(workqueue, graph);
		}
	}

	cpset_t done_set;
	cpset_init(&done_set, hash_ptr, ptr_equals);
	while (!pdeq_empty(workqueue)) {
		ir_graph *g = pdeq_getl(workqueue);
		assert(is_ir_graph(g));

		if (cpset_find(&done_set, g) != NULL) continue;

		printf("\n== %s.%s\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)));

		cpset_insert(&done_set, g); // mark as done _before_ walking because of possible recursive calls !??? -> not necessary but not wrong!? TODO
		irg_walk_graph(g, NULL, walk_callgraph_and_devirtualize, &env);
	}

	// free data structures
	del_pdeq(workqueue);
	cpset_destroy(&done_set);

}


void rta_optimization(size_t n_entry_points, ir_entity **entry_points) {
	cpset_t entry_points_set;
	cpset_init(&entry_points_set, hash_ptr, ptr_equals);
	for (size_t i=0; i<n_entry_points; i++) {
		ir_entity *entry_point = entry_points[i];
		cpset_insert(&entry_points_set, entry_point);
	}

	cpset_t used_classes;
	cpset_t used_methods;
	cpmap_t dyncall_targets;

	rta_run(&entry_points_set, &used_classes, &used_methods, &dyncall_targets);
	rta_devirtualize_calls(&entry_points_set, &dyncall_targets);
	//rta_discard_unused(&used_classes, &used_methods); ??
	rta_dispose_results(&used_classes, &used_methods, &dyncall_targets);
}
