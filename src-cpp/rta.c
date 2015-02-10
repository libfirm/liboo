/*
 * This file is part of liboo.
 */

/**
 * @file	rta.c
 * @brief	Devirtualization of dynamically bound calls through Rapid Type Analysis
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


// debug setting
#define DEBUG_RTA 0
#define DEBUGOUT if (DEBUG_RTA) printf

// override option just for early development to keep going without information about live classes
#define JUST_CHA 0


#define is_inherited_copy(X) (oo_get_method_is_inherited(X) && ddispatch_get_bound_entity(X) != X)


static ir_entity *get_class_member_by_name(ir_type *cls, ident *ident) // function which was removed from newer libfirm versions
{
	for (size_t i = 0, n = get_class_n_members(cls); i < n; ++i) {
		ir_entity *entity = get_class_member(cls, i);
		if (get_entity_ident(entity) == ident)
			return entity;
	}
	return NULL;
}

static int ptr_equals(const void *pt1, const void *pt2) // missing default pointer compare function
{
	return pt1 == pt2;
}

static inline cpmap_t *new_cpmap(cpmap_hash_function hash_function, cpmap_cmp_function cmp_function) // missing new function for cpmap
{
	cpmap_t *cpmap = (cpmap_t*)malloc(sizeof(cpmap_t));
	cpmap_init(cpmap, hash_function, cmp_function);
	return cpmap;
}

static inline cpset_t *new_cpset(cpset_hash_function hash_function, cpset_cmp_function cmp_function) // missing new function for cpset
{
	cpset_t *cpset = (cpset_t*)malloc(sizeof(cpset_t));
	cpset_init(cpset, hash_function, cmp_function);
	return cpset;
}
/*
static inline cpmap_iterator_t *new_cpmap_iterator(cpmap_t *map) // missing new function for cpmap iterator
{
	cpmap_iterator_t *it = (cpmap_iterator_t*)malloc(sizeof(cpmap_iterator_t));
	cpmap_iterator_init(it, map);
	return it;
}

static inline cpset_iterator_t *new_cpset_iterator(cpset_t *set) // missing new function for cpset iterator
{
	cpset_iterator_t *it = (cpset_iterator_t*)malloc(sizeof(cpset_iterator_t));
	cpset_iterator_init(it, set);
	return it;
}
*/


static ir_entity *default_detect_call(ir_node *call) { (void)call; return NULL; }

static ir_entity *(*detect_call)(ir_node *call) = &default_detect_call;

void rta_set_detection_callbacks(ir_entity *(*detect_call_callback)(ir_node *call))
{
	assert(detect_call_callback);
	detect_call = detect_call_callback;
}


typedef struct analyzer_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpset_t *in_queue; // set to avoid duplicates in the workqueue
	cpset_t *done_set; // set to mark graphs that were already analyzed
	cpset_t *live_classes; // live classes found by examining object creation (external classes are left out and always considered as live)
	cpset_t *live_methods; // live method entities
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically bound call (Map: call entity -> Set: method entities)
	cpmap_t *unused_targets; // map that stores a map for every class which stores unused potential call targets of dynamic calls and a set of the call entities that would call them if the class were live) (Map: class -> (Map: method entity -> Set: call entities)) This is needed to patch analysis result when a class becomes live after there were already some dynamically bound calls that would call a method of it.
	cpset_t *external_methods; // set that stores already handled external methods (just for not handling the same method more than once)
} analyzer_env;


static void add_to_workqueue(ir_entity *method, analyzer_env *env); // forward declaration


static void check_for_external_superclasses_recursive(ir_type *klass, ir_type* superclass, analyzer_env *env)
{
	assert(is_Class_type(klass));
	assert(is_Class_type(superclass));
	assert(env);

	DEBUGOUT("\t\t\t\t\t\t\tchecking superclass %s of %s\n", get_class_name(superclass), get_class_name(klass));
	if (oo_get_class_is_extern(superclass)) { // if extern
		DEBUGOUT("\t\t\t\t\t\t\tfound external superclass %s of %s\n", get_class_name(superclass), get_class_name(klass));
		// add all methods of superclass that were overwritten by klass to workqueue because they could be called by external code
		for (size_t i=0, n=get_class_n_members(superclass); i<n; i++) {
			ir_entity *member = get_class_member(superclass, i);
			if (!is_method_entity(member)) continue;
			if (oo_get_method_is_final(member)) continue;
			ir_entity *overwriting = get_class_member_by_name(klass, get_entity_ident(member)); // note: This only works because whole signature is already encoded in entity name!
			if (overwriting != NULL && !is_inherited_copy(overwriting)) { //FIXME constructors should be skipped but no frontend independent notion of constructors in liboo
				cpset_insert(env->live_methods, overwriting);
				add_to_workqueue(overwriting, env);
			}
		}
	}

	size_t n = get_class_n_supertypes(superclass);
	DEBUGOUT("\t\t\t\t\t\t\t\t%s has %lu superclasses\n", get_class_name(superclass), (unsigned long)n);
	for (size_t i=0; i<n; i++) {
		ir_type *sc = get_class_supertype(superclass, i);
		check_for_external_superclasses_recursive(klass, sc, env);
	}
}

static void check_for_external_superclasses(ir_type *klass, analyzer_env *env)
{
	assert(is_Class_type(klass));
	assert(env);

	if (oo_get_class_is_extern(klass)) return;

	DEBUGOUT("\t\t\t\t\t\tchecking for external superclasses of %s\n", get_class_name(klass));
	size_t n = get_class_n_supertypes(klass);
	DEBUGOUT("\t\t\t\t\t\t\t%s has %lu superclasses\n", get_class_name(klass), (unsigned long)n);
	for (size_t i=0; i<n; i++) {
		ir_type *superclass = get_class_supertype(klass, i);
		check_for_external_superclasses_recursive(klass, superclass, env);
	}
}

// add method entity to target sets of all call entities
static void add_to_dyncalls(ir_entity *method, cpset_t *call_entities, analyzer_env *env)
{
	assert(is_method_entity(method));
	assert(call_entities);
	assert(env);

	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, call_entities);
	ir_entity *call_entity;
	while ((call_entity = cpset_iterator_next(&iterator)) != NULL) {
		cpset_t *targets = cpmap_find(env->dyncall_targets, call_entity);
		assert(targets != NULL);
		//assert(cpset_find(targets, method) == NULL); // doesn't make sense!?

		DEBUGOUT("\t\t\t\t\tupdating method %s.%s for call %s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity));
		// add to targets set
		cpset_insert(targets, method);

		// add to live methods
		cpset_insert(env->live_methods, method);

		// add to workqueue
		add_to_workqueue(method, env);
	}
}

static void add_new_live_class(ir_type *klass, analyzer_env *env)
{
	assert(is_Class_type(klass));
	assert(env);

	if (cpset_find(env->live_classes, klass) == NULL // if it had not already been added
	    && !oo_get_class_is_extern(klass) && !oo_get_class_is_abstract(klass)) { // if not extern and not abstract
		// add to live classes
		cpset_insert(env->live_classes, klass);
		DEBUGOUT("\t\t\t\t\tadded new live class %s\n", get_class_name(klass));

		// update existing results
		cpmap_t *methods = cpmap_find(env->unused_targets, klass);
		if (methods != NULL) {
			{
				cpmap_iterator_t iterator;
				cpmap_iterator_init(&iterator, methods);
				cpmap_entry_t *entry;
				while ((entry = cpmap_iterator_next(&iterator))->key != NULL || entry->data != NULL) {
					ir_entity *method = (ir_entity*)entry->key;
					cpset_t *call_entities = entry->data;

					add_to_dyncalls(method, call_entities, env);

					cpmap_remove_iterator(methods, &iterator);
					cpset_destroy(call_entities);
					free(call_entities);
				}
			}
			cpmap_remove(env->unused_targets, klass);
			cpmap_destroy(methods);
			free(methods);
		}

		check_for_external_superclasses(klass, env);
	}
}

static void memorize_unused_target(ir_type *klass, ir_entity *entity, ir_entity *call_entity, analyzer_env *env)
{
	assert(is_Class_type(klass));
	assert(is_method_entity(entity));
	assert(is_method_entity(call_entity));
	assert(env);

	cpmap_t *methods = cpmap_find(env->unused_targets, klass);
	if (methods == NULL) {
		methods = new_cpmap(hash_ptr, ptr_equals);
		cpmap_set(env->unused_targets, klass, methods);
	}
	cpset_t *call_entities = cpmap_find(methods, entity);
	if (call_entities == NULL) {
		call_entities = new_cpset(hash_ptr, ptr_equals);
		cpmap_set(methods, entity, call_entities);
	}
	cpset_insert(call_entities, call_entity);
}

static void handle_external_method(ir_entity *method, analyzer_env *env)
{
	assert(is_method_entity(method));
	assert(env);

	assert(!is_inherited_copy(method));
	if (cpset_find(env->external_methods, method) == NULL) { // check if not already handled this method
		DEBUGOUT("\t\t\thandling external method %s.%s ( %s )\n", get_class_name(get_entity_owner(method)), get_entity_name(method), get_entity_ld_name(method));

		// something to do?

		cpset_insert(env->external_methods, method);
	}
}

static void add_to_workqueue(ir_entity *method, analyzer_env *env)
{
	assert(is_method_entity(method));
	assert(env);

	assert(!is_inherited_copy(method));
	ir_graph *graph = get_entity_irg(method);
	if (graph) {
		if (cpset_find(env->done_set, graph) == NULL && cpset_find(env->in_queue, graph) == NULL) { // only enqueue if not already done or enqueued
			DEBUGOUT("\t\t\tadding %s.%s to workqueue\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			pdeq_putr(env->workqueue, graph);
			cpset_insert(env->in_queue, graph);
		}
	} else {
		// treat methods without graph as external methods
		// cannot distinguish here between real external functions (usually C functions) and other cases in which the entity has no graph
		DEBUGOUT("\t\t\tfound no graph, probably external\n");
		handle_external_method(method, env);
	}
}

static void take_entity(ir_entity *entity, cpset_t *result_set, analyzer_env *env)
{
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	if (cpset_find(result_set, entity) == NULL) { // take each entity only once (the sets won't mind but the workqueue)
		DEBUGOUT("\t\t\ttaking entity %s.%s ( %s )\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));

		// add to live methods
		cpset_insert(env->live_methods, entity);

		// add to result set
		cpset_insert(result_set, entity);

		// add to workqueue
		add_to_workqueue(entity, env);
	}
}

static void collect_methods_recursive(ir_entity *call_entity, ir_type *klass, ir_entity *entity, cpset_t *result_set, analyzer_env *env)
{
	assert(is_method_entity(call_entity));
	assert(is_Class_type(klass));
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	DEBUGOUT("\t\twalking class %s\n", get_class_name(klass));
	ir_entity *current_entity = entity;
	ir_entity *overwriting_entity = get_class_member_by_name(klass, get_entity_ident(current_entity)); // note: This only works because whole signature is already encoded in entity name!
	if (overwriting_entity != NULL && overwriting_entity != current_entity) { // if has overwriting entity
		DEBUGOUT("\t\t\t%s.%s overwrites %s.%s\n", get_class_name(get_entity_owner(overwriting_entity)), get_entity_name(overwriting_entity), get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity));

		current_entity = overwriting_entity;
	}
	//else // inherited

	while (is_inherited_copy(current_entity)) { // use copied entities of inherited methods to find implementations (especially in the case when interface method is implemented by a superclass)
		ir_entity *impl_entity = ddispatch_get_bound_entity(current_entity);
		assert(impl_entity);
		assert(!oo_get_class_is_interface(get_entity_owner(impl_entity)));
		assert(!oo_get_method_is_abstract(impl_entity));
		DEBUGOUT("\t\t\tfound copied method entity %s.%s -> %s.%s\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity), get_class_name(get_entity_owner(impl_entity)), get_entity_name(impl_entity));

		current_entity = impl_entity;
	}

	if (!oo_get_method_is_abstract(current_entity)) { // ignore abstract methods
		if (cpset_find(env->live_classes, klass) != NULL || oo_get_class_is_extern(klass) || JUST_CHA) { // if class is considered in use
			take_entity(current_entity, result_set, env);
		} else {
			DEBUGOUT("\t\t\tclass not in use, memorizing %s.%s %s\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity), ((get_entity_irg(current_entity)) ? "G" : "N"));
			memorize_unused_target(klass, current_entity, call_entity, env); // remember entity with this class for patching if this class will become used
		}
	} else
		DEBUGOUT("\t\t\t%s.%s is abstract\n", get_class_name(get_entity_owner(current_entity)), get_entity_name(current_entity));

	size_t n_subtypes = get_class_n_subtypes(klass);
	DEBUGOUT("\t\t\t%s has %lu subclasses\n", get_class_name(klass), (unsigned long)n_subtypes);
	for (size_t i=0; i<n_subtypes; i++) {
		ir_type *subclass = get_class_subtype(klass, i);

		collect_methods_recursive(call_entity, subclass, current_entity, result_set, env);
	}
}

// collect method entities from downwards in the class hierarchy
// it walks down the classes to have the entities with the classes even when the method is inherited
static void collect_methods(ir_entity *call_entity, cpset_t *result_set, analyzer_env *env)
{
	collect_methods_recursive(call_entity, get_entity_owner(call_entity), call_entity, result_set, env);
}

static void analyzer_handle_static_call(ir_node *call, ir_entity *entity, analyzer_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	DEBUGOUT("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
	//DEBUGOUT("\t\t\toverwrites: %u\n", get_entity_n_overwrites(entity));
	//DEBUGOUT("\t\t\toverwrittenby: %u\n", get_entity_n_overwrittenby(entity));
	//DEBUGOUT("\t\t\tis abstract method: %u\n", oo_get_method_is_abstract(entity));
	//DEBUGOUT("\t\t\tis interface method: %u\n", oo_get_class_is_interface(get_entity_owner(entity)));
	//DEBUGOUT("\t\t\tis owner extern: %u\n", oo_get_class_is_extern(get_entity_owner(entity)));

	// add to live methods
	cpset_insert(env->live_methods, entity);

	add_to_workqueue(entity, env);


	// hack to detect calls (like class initialization) that are hidden in frontend-specific nodes
	ir_graph *graph = get_entity_irg(entity);
	if (!graph) {
		// ask frontend if there are additional methods called here (e.g. needed to detect class initialization)
		ir_entity *called_method = detect_call(call); //TODO support for more than one
		if (called_method) {
			assert(is_method_entity(called_method));
			//assert(get_entity_irg(called_method)); // can be external
			DEBUGOUT("\t\texternal method calls %s.%s ( %s )\n", get_class_name(get_entity_owner(called_method)), get_entity_name(called_method), get_entity_ld_name(called_method));
			cpset_insert(env->live_methods, called_method);
			add_to_workqueue(called_method, env);
		}
	}
}

static void analyzer_handle_dynamic_call(ir_node *call, ir_entity *entity, analyzer_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	DEBUGOUT("\tdynamic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
	//DEBUGOUT("\t\t\toverwrites: %u\n", get_entity_n_overwrites(entity));
	//DEBUGOUT("\t\t\toverwrittenby: %u\n", get_entity_n_overwrittenby(entity));
	//DEBUGOUT("\t\t\tis abstract method: %u\n", oo_get_method_is_abstract(entity));
	//DEBUGOUT("\t\t\tis interface method: %u\n", oo_get_class_is_interface(get_entity_owner(entity)));
	//DEBUGOUT("\t\t\tis owner extern: %u\n", oo_get_class_is_extern(get_entity_owner(entity)));

	if (cpmap_find(env->dyncall_targets, entity) == NULL) { // if not already done
		// calculate set of all method entities that this call could potentially call

		// first static lookup upwards in the class hierarchy for the case of an inherited method
		// The entity from the MethodSel node is already what the result of s static lookup would be or it is an inherited copy which points to the inherited method.

		// then collect all potentially called method entities from downwards the class hierarchy
		cpset_t *result_set = new_cpset(hash_ptr, ptr_equals);
		collect_methods(entity, result_set, env); // note: This should work correctly with inherited copies and interface methods.

		// note: cannot check for nonempty result set here because classes could be nonlive at this point but become live later depending on the order in which methods are analyzed

		cpmap_set(env->dyncall_targets, entity, result_set);
	}
}

static void walk_callgraph_and_analyze(ir_node *node, void *environment)
{
	assert(environment);
	analyzer_env *env = (analyzer_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Address: {
		ir_node *address = node;
		ir_entity *entity = get_Address_entity(address);
		if (is_method_entity(entity)) {
			// could be a function whose address is taken (although usually the Address node of a normal call, these cases cannot be distinguished)
			DEBUGOUT("\tAddress with method entity: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
			DEBUGOUT("\t\tcould be address taken, so it could be called\n");

			// add to live methods
			cpset_insert(env->live_methods, entity);

			add_to_workqueue(entity, env);
		}
		break;
	}
	case iro_Call: {
		ir_node *call = node;
		ir_node *callee = get_irn_n(call, 1);
		if (is_Address(callee)) {
			// static call
			ir_entity *entity = get_Address_entity(callee);

			analyzer_handle_static_call(call, entity, env);

		} else if (is_Proj(callee)) {
			ir_node *pred = get_Proj_pred(callee);
			if (is_MethodSel(pred)) {
				ir_node *methodsel = pred;
				ir_entity *entity = get_MethodSel_entity(methodsel);

				if (oo_get_call_is_statically_bound(call)) {
					// weird case of Call with MethodSel that is marked statically bound
					analyzer_handle_static_call(call, entity, env);
				} else {
					// dynamic call
					analyzer_handle_dynamic_call(call, entity, env);
				}
			} else {
				// function pointers currently not supported
				DEBUGOUT("\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
				DEBUGOUT("-> %s", gdb_node_helper(callee));
				DEBUGOUT("-> %s\n", gdb_node_helper(pred));
			}
		} else {
			// function pointers currently not supported
			DEBUGOUT("\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
			DEBUGOUT("-> %s\n", gdb_node_helper(callee));
		}
		break;
	}
	default:
		if (is_VptrIsSet(node)) {
			// use new VptrIsSet node for detection of object creation
			ir_type *klass = get_VptrIsSet_type(node);
			assert(is_Class_type(klass));

			DEBUGOUT("\tVptrIsSet: %s\n", get_class_name(klass));
			add_new_live_class(klass, env);
		}
		// skip other node types
		break;
	}
}


/** run Rapid Type Analysis
 * It runs over a reduced callgraph and detects which classes and methods are actually used and computes reduced sets of potentially called targets for each dynamically bound call.
 * @note RTA must know of _all_ definitely executed code parts (main, static sections, global contructors or all nonprivate functions if it's a library)! It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!!
 * @note RTA also won't work with programs that dynamically load classes at run-time! It can lead to defective programs!!
 * @note RTA might also produce incorrect programs if there is some Java Reflections shenanigans in the code (internal or external), especially using java.lang.Class.newInstance()
 * @note Give classes that are instantiated in native methods of a nonexternal standard library or runtime as initial_live_classes, give methods called in these native methods as additional entry points. If something is missing, RTA could produce incorrect programs! This also means that native methods in the program that do arbitrary things are not supported.
 * @param entry_points NULL-terminated array of method entities, give all entry points to program code, may _not_ be NULL and must contain at least one method entity, also all entry points should have a graph
 * @param initial_live_classes NULL-terminated array of classes that should always be considered live, may be NULL
 * @param live_classes give pointer to empty uninitialized set for receiving results, This is where all live classes are put (as ir_type*).
 * @param live_methods give pointer to empty uninitialized set for receiving results, This is where all live methods are put (as ir_entity*).
 * @param dyncall_targets give pointer to empty uninitialized map for receiving results, This is where call entities are mapped to their actually used potential call targets (ir_entity* -> {ir_entity*}). It's used to optimize dynamically bound calls if possible. (see also function rta_optimize_dyncalls)
 */
static void rta_run(ir_entity **entry_points, ir_type **initial_live_classes, cpset_t *live_classes, cpset_t *live_methods, cpmap_t *dyncall_targets)
{
	assert(entry_points);
	assert(live_classes);
	assert(live_methods);
	assert(dyncall_targets);

	cpset_init(live_classes, hash_ptr, ptr_equals);
	cpset_init(live_methods, hash_ptr, ptr_equals);
	cpmap_init(dyncall_targets, hash_ptr, ptr_equals);

	cpmap_t unused_targets;
	cpmap_init(&unused_targets, hash_ptr, ptr_equals);

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
		.live_classes = live_classes,
		.live_methods = live_methods,
		.dyncall_targets = dyncall_targets,
		.unused_targets = &unused_targets,
		.external_methods = &external_methods
	};

	{ // add all given entry points to live methods and to workqueue
		ir_entity *entry;
		size_t i = 0;
		DEBUGOUT("entrypoints:\n");
		for (; (entry = entry_points[i]) != NULL; i++) {
			assert(is_method_entity(entry));
			DEBUGOUT("\t%s\n", get_entity_name(entry));
			cpset_insert(live_methods, entry);
			// add to workqueue
			ir_graph *graph = get_entity_irg(entry);
			assert(graph); // don't give methods without a graph as entry points for the analysis !? TODO
			if (cpset_find(&in_queue, graph) == NULL) {
				pdeq_putr(workqueue, graph);
				cpset_insert(&in_queue, graph);
			}
		}
		assert(i > 0 && "give at least one entry point");
	}

	// add all given initial live classes to live classes
	if (initial_live_classes != NULL) {
		DEBUGOUT("\ninitial live classes:\n");
		ir_type *entry;
		for (size_t i=0; (entry = initial_live_classes[i]) != NULL; i++) {
			assert(is_Class_type(entry));
			DEBUGOUT("\t%s\n", get_class_name(entry));
			cpset_insert(live_classes, entry);
			check_for_external_superclasses(entry, &env);
		}
	}

	while (!pdeq_empty(workqueue)) {
		ir_graph *g = pdeq_getl(workqueue);
		assert(g && get_kind(g) == k_ir_graph);
		cpset_remove(&in_queue, g);

		if (cpset_find(&done_set, g) != NULL) continue;

		DEBUGOUT("\n== %s.%s (%s)\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)), get_entity_ld_name(get_irg_entity(g)));

		cpset_insert(&done_set, g); // mark as done _before_ walking to not add it again in case of recursive calls
		irg_walk_graph(g, NULL, walk_callgraph_and_analyze, &env);
	}
	assert(cpset_size(&in_queue) == 0);


	if (DEBUG_RTA) {
		DEBUGOUT("\n\n==== Results ==============================================\n");
		{
			DEBUGOUT("\nlive classes:\n");
			cpset_iterator_t iterator;
			cpset_iterator_init(&iterator, live_classes);
			ir_type *klass;
			while ((klass = cpset_iterator_next(&iterator)) != NULL) {
				DEBUGOUT("\t%s\n", get_class_name(klass));
			}
		}
		{
			DEBUGOUT("\nlive methods:\n");
			cpset_iterator_t iterator;
			cpset_iterator_init(&iterator, live_methods);
			ir_entity *method;
			while ((method = cpset_iterator_next(&iterator)) != NULL) {
				//DEBUGOUT("\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
				DEBUGOUT("\t%s.%s\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			}
		}
		{
			DEBUGOUT("\ndyncall target sets:\n");
			//DEBUGOUT("size %u\n", cpmap_size(dyncall_targets));
			cpmap_iterator_t iterator;
			cpmap_iterator_init(&iterator, dyncall_targets);
			cpmap_entry_t *entry;
			while ((entry = cpmap_iterator_next(&iterator))->key != NULL || entry->data != NULL) {
				const ir_entity *call_entity = entry->key;
				assert(call_entity);
				DEBUGOUT("\t%s.%s %s\n", get_class_name(get_entity_owner(call_entity)), get_entity_name(call_entity), (oo_get_class_is_extern(get_entity_owner(call_entity))) ? "external" : "");

				cpset_t *targets = entry->data;
				assert(targets);
				cpset_iterator_t it;
				cpset_iterator_init(&it, targets);
				ir_entity *method;
				while ((method = cpset_iterator_next(&it)) != NULL) {
					//DEBUGOUT("\t\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
					DEBUGOUT("\t\t%s.%s %s\n", get_class_name(get_entity_owner(method)), get_entity_name(method), (oo_get_class_is_extern(get_entity_owner(call_entity))) ? "external" : "");
				}
			}
		}
		DEBUGOUT("\n=============================================================\n");
	}

	// free data structures
	del_pdeq(workqueue);
	cpset_destroy(&in_queue);
	cpset_destroy(&done_set);

	{ // delete the maps and sets in map unused_targets
		cpmap_iterator_t iterator;
		cpmap_iterator_init(&iterator, &unused_targets);
		cpmap_entry_t *entry;
		while ((entry = cpmap_iterator_next(&iterator))->key != NULL || entry->data != NULL) {
			cpmap_t* map = entry->data;
			assert(map);

			cpmap_iterator_t inner_iterator;
			cpmap_iterator_init(&inner_iterator, map);
			cpmap_entry_t *inner_entry;
			while ((inner_entry = cpmap_iterator_next(&inner_iterator))->key != NULL || inner_entry->data != NULL) {
				cpset_t *set = inner_entry->data;
				assert(set);

				cpmap_remove_iterator(map, &inner_iterator);
				cpset_destroy(set);
				free(set);
			}
			cpmap_remove_iterator(&unused_targets, &iterator);
			cpmap_destroy(map);
			free(map);
		}
	}
	cpmap_destroy(&unused_targets);
	cpset_destroy(&external_methods);

	// /!\ live_classes, live_methods and dyncall_targets are given from outside and return the results, but the sets in map dyncall_targets are allocated in the process and have to be deleted later

}


/** frees memory allocated for the results returned by function run_rta
 * @note does not free the memory of the sets and maps themselves, just their content allocated during RTA
 * @param live_classes as returned by rta_run
 * @param live_methods as returned by rta_run
 * @param dyncall_targets as returned by rta_run
 */
static void rta_dispose_results(cpset_t *live_classes, cpset_t *live_methods, cpmap_t *dyncall_targets)
{
	assert(live_classes);
	assert(live_methods);
	assert(dyncall_targets);

	cpset_destroy(live_classes);
	cpset_destroy(live_methods);

	// delete the sets in map dyncall_targets
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
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically bound call (Map: call entity -> Set: method entities)
} optimizer_env;

static void optimizer_add_to_workqueue(ir_entity *method, optimizer_env *env)
{
	assert(is_method_entity(method));
	assert(env);

	ir_graph *graph = get_entity_irg(method);
	if (graph) {
		if (cpset_find(env->done_set, graph) == NULL && cpset_find(env->in_queue, graph) == NULL) { // only enqueue if not already done or enqueued
			DEBUGOUT("\t\tadding %s.%s to workqueue\n", get_class_name(get_entity_owner(method)), get_entity_name(method));
			pdeq_putr(env->workqueue, graph);
			cpset_insert(env->in_queue, graph);
		}
	}
}

static void optimizer_handle_static_call(ir_node *call, ir_entity *entity, optimizer_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	DEBUGOUT("\tstatic call: %s.%s %s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

	optimizer_add_to_workqueue(entity, env);


	// hack to detect calls (like class initialization) that are hidden in frontend-specific nodes
	ir_graph *graph = get_entity_irg(entity);
	if (!graph) {
		ir_entity *called_method = detect_call(call);
		if (called_method) {
			assert(is_method_entity(called_method));
			//assert(get_entity_irg(called_method)); // can be external
			DEBUGOUT("\t\texternal method calls %s.%s (%s)\n", get_class_name(get_entity_owner(called_method)), get_entity_name(called_method), get_entity_ld_name(called_method));
			optimizer_add_to_workqueue(called_method, env);
		}
	}
}

static void optimizer_handle_dynamic_call(ir_node *call, ir_entity *entity, ir_node *methodsel, optimizer_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(methodsel);
	assert(is_MethodSel(methodsel));
	assert(env);

	ir_type *owner = get_entity_owner(entity);
	DEBUGOUT("\tdynamic call: %s.%s %s\n", get_class_name(owner), get_entity_name(entity), gdb_node_helper(entity));

	cpset_t *targets = cpmap_find(env->dyncall_targets, entity);
	assert(targets);
	// note: cannot check for nonempty target set here because there can be legal programs that have calls with empty target sets although they will probably run into an exception when executed! (e.g. interface call without implementing class and program initializes reference to null, actually same with abstract class or nonlive class)

	if (cpset_size(targets) == 1 && (!oo_get_class_is_extern(owner) || oo_get_class_is_final(owner) || oo_get_method_is_final(entity))) { // exactly one target and not extern nonfinal
		// devirtualize call
		cpset_iterator_t it;
		cpset_iterator_init(&it, targets);
		ir_entity *target = cpset_iterator_next(&it);
		assert(cpset_iterator_next(&it) == NULL);

		// set an Address node as callee to make the call statically bound
		DEBUGOUT("\t\tdevirtualizing call %s.%s -> %s.%s\n", get_class_name(get_entity_owner(entity)), get_entity_name(entity), get_class_name(get_entity_owner(target)), get_entity_name(target));
		ir_graph *graph = get_irn_irg(methodsel);
		ir_node *address = new_r_Address(graph, target);
		ir_node *mem = get_irn_n(methodsel, 0);
		ir_node *input[] = { mem, address };
		turn_into_tuple(methodsel, 2, input);
	}

	// add to workqueue
	cpset_iterator_t it;
	cpset_iterator_init(&it, targets);
	ir_entity *target;
	while ((target = cpset_iterator_next(&it)) != NULL) {
		optimizer_add_to_workqueue(target, env);
	}
}

static void walk_callgraph_and_devirtualize(ir_node *node, void* environment)
{
	assert(environment);
	optimizer_env *env = (optimizer_env*)environment;

	switch (get_irn_opcode(node)) {
	case iro_Call: {
		ir_node *call = node;
		ir_node *callee = get_irn_n(call, 1);
		if (is_Address(callee)) {
			// static call
			ir_entity *entity = get_Address_entity(callee);

			optimizer_handle_static_call(call, entity, env);

		} else if (is_Proj(callee)) {
			ir_node *pred = get_Proj_pred(callee);
			if (is_MethodSel(pred)) {
				ir_node *methodsel = pred;
				ir_entity *entity = get_MethodSel_entity(methodsel);

				if (oo_get_call_is_statically_bound(call)) {
					// weird case of Call with MethodSel that is marked statically bound
					optimizer_handle_static_call(call, entity, env);
				} else {
					// dynamic call
					optimizer_handle_dynamic_call(call, entity, methodsel, env);
				}
			} else {
				// function pointers currently not supported
				DEBUGOUT("\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
				DEBUGOUT("-> %s", gdb_node_helper(callee));
				DEBUGOUT("-> %s\n", gdb_node_helper(pred));
			}
		} else {
			// function pointers currently not supported
			DEBUGOUT("\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
			DEBUGOUT("-> %s\n", gdb_node_helper(callee));
		}
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
static void rta_devirtualize_calls(ir_entity **entry_points, cpmap_t *dyncall_targets)
{
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
			assert(graph); // don't give methods without a graph as entry points for the analysis !? TODO
			if (cpset_find(&in_queue, graph) == NULL) {
				pdeq_putr(workqueue, graph);
				cpset_insert(&in_queue, graph);
			}
		}
	}

	while (!pdeq_empty(workqueue)) {
		ir_graph *g = pdeq_getl(workqueue);
		assert(g && get_kind(g) == k_ir_graph);
		cpset_remove(&in_queue, g);

		if (cpset_find(&done_set, g) != NULL) continue;

		DEBUGOUT("\n== %s.%s (%s)\n", get_class_name(get_entity_owner(get_irg_entity(g))), get_entity_name(get_irg_entity(g)), get_entity_ld_name(get_irg_entity(g)));

		cpset_insert(&done_set, g); // mark as done _before_ walking to not add it again in case of recursive calls
		irg_walk_graph(g, NULL, walk_callgraph_and_devirtualize, &env);
	}
	assert(cpset_size(&in_queue) == 0);

	// free data structures
	del_pdeq(workqueue);
	cpset_destroy(&in_queue);
	cpset_destroy(&done_set);
}


void rta_optimization(ir_entity **entry_points, ir_type **initial_live_classes)
{
	assert(entry_points);

	cpset_t live_classes;
	cpset_t live_methods;
	cpmap_t dyncall_targets;

	rta_run(entry_points, initial_live_classes, &live_classes, &live_methods, &dyncall_targets);
	rta_devirtualize_calls(entry_points, &dyncall_targets);
	//rta_discard(&live_classes, &live_methods); ??
	rta_dispose_results(&live_classes, &live_methods, &dyncall_targets);
}
