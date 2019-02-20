/*
 * This file is part of liboo and contains the implementation
 * of XTA to devirtualize dynamic calls.
 */

/**
 * @file	xta.c
 * @brief	Devirtualization of dynamically bound calls through Hyprid Type Analysis
 * @author	Daniel Biester
 * @date	2018
 */


#include "liboo/xta.h"
#include <assert.h>

#include <stdbool.h>
#include <string.h>

#include <liboo/oo.h>
#include <liboo/nodes.h>

#include "adt/cpmap.h"
#include "adt/cpset.h"
#include "adt/pdeq.h"
#include "adt/hashptr.h"

#include <libfirm/iroptimize.h>
#include <libfirm/timing.h>
// debug setting
#define DEBUG_XTA 2
#define DEBUGOUT(lvl, ...) if (DEBUG_XTA >= lvl) printf(__VA_ARGS__);
#define DEBUGCALL(lvl) if (DEBUG_XTA >= lvl)
// stats
#define XTA_STATS 1
//#undef XTA_STATS // comment to activate stats

static void make_subset(cpset_t *a, cpset_t *b);
static void cut_sets(cpset_t *res, cpset_t *a, cpset_t *b);

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
	cpmap_t *cpmap = (cpmap_t *)malloc(sizeof(cpmap_t));
	cpmap_init(cpmap, hash_function, cmp_function);
	return cpmap;
}

static inline cpset_t *new_cpset(cpset_hash_function hash_function, cpset_cmp_function cmp_function) // missing new function for cpset
{
	cpset_t *cpset = (cpset_t *)malloc(sizeof(cpset_t));
	cpset_init(cpset, hash_function, cmp_function);
	return cpset;
}

static ir_entity *default_detect_call(ir_node *call)
{
	(void)call;
	return NULL;
}

static ir_entity *(*detect_call)(ir_node *call) = &default_detect_call;

void xta_set_detection_callbacks(ir_entity * (*detect_call_callback)(ir_node *call))
{
	assert(detect_call_callback);
	detect_call = detect_call_callback;
}

static bool default_is_constr(ir_entity *method)
{
	(void)method;
	return false;
}

static bool (*is_constr)(ir_entity *method) = &default_is_constr;

void xta_set_is_constructor_callback(bool (*is_constr_callback)(ir_entity *method))
{
	assert(is_constr_callback);
	is_constr = is_constr_callback;
}

static cpset_t *non_leaking_ext_methods = NULL;

void xta_set_non_leaking_ext_methods(cpset_t *non_leak_meth)
{
	non_leaking_ext_methods = non_leak_meth;
}

typedef struct iteration_set {
	cpset_t *propagated;
	cpset_t *current;
	cpset_t *new;
} iteration_set;

typedef struct analyzer_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpmap_t *done_map; // map to mark methods that were already analyzed. Maps entities to their method_info
	cpmap_t *live_classes_fields; //Live classes for field entities

	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically bound call (Map: call entity -> caller -> Set: method entities)
	cpmap_t *unused_targets; // map that stores a map for evert call site which stores a map for every class which stores unused potential call targets of dynamic calls and a set of the call entities that would call them if the class were live) (Map: call site ->(Map: class -> (Map: method entity -> Set: call entities)) This is needed to update results when a class becomes live after there were already some dynamically bound calls that would call a method of it.

	cpmap_t *ext_called_constr; //constructors called in methods without graph. Map method to set of class types
	cpset_t *ext_live_classes; //classes which are in use in external code
	cpset_t *methods_ext_called; //All application methods which can be called by a library (in theory)
} analyzer_env;

typedef struct method_info {
	bool is_library_method; //True if library method. Then field live_classes is not used, instead ext_live_classes of analyzer_env is used.

	cpset_t *callers; //callers of this method
	iteration_set *live_classes; //classes which are marked as live in this method
	cpset_t *parameter_subtypes; //parameter types and subtypes
	cpset_t *return_subtypes; //return types and subtypes
	cpset_t *field_writes; //entities of fields that are written to in this method
	cpset_t *field_reads; //entities of fields that are read from in this method
} method_info;

typedef struct method_env {
	analyzer_env *analyzer_env; //"global state"
	ir_entity *entity; //method entity
	method_info *info; //the metadata struct of the method entity
} method_env;

static inline iteration_set *new_iteration_set(void)
{

	iteration_set *i_set = (iteration_set *) malloc(sizeof(iteration_set));

	cpset_t *propagated = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(propagated, hash_ptr, ptr_equals);
	i_set->propagated = propagated;

	cpset_t *current = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(current, hash_ptr, ptr_equals);
	i_set->current = current;

	cpset_t *new = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(new, hash_ptr, ptr_equals);
	i_set->new = new;

	return i_set;
}

static inline method_info *new_method_info(void)
{
	method_info *info = (method_info *) malloc(sizeof(method_info));

	info->is_library_method = false;

	cpset_t *callers = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(callers, hash_ptr, ptr_equals);
	info->callers = callers;

	info->live_classes = new_iteration_set();

	cpset_t *parameter_subtypes = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(parameter_subtypes, hash_ptr, ptr_equals);
	info->parameter_subtypes = parameter_subtypes;

	cpset_t *return_subtypes = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(return_subtypes, hash_ptr, ptr_equals);
	info->return_subtypes = return_subtypes;

	cpset_t *field_writes = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(field_writes, hash_ptr, ptr_equals);
	info->field_writes = field_writes;

	cpset_t *field_reads = (cpset_t *) malloc(sizeof(cpset_t));
	cpset_init(field_reads, hash_ptr, ptr_equals);
	info->field_reads = field_reads;

	return info;
}

static bool ext_method_is_non_leaking(ir_entity *method)
{
	if (non_leaking_ext_methods == NULL)
		return false;
	return cpset_find(non_leaking_ext_methods, method);
}

static void add_to_workqueue(ir_entity *method, ir_type *klass, method_env *env); // forward declaration

// add method entity to target sets of all call entities
static void add_to_dyncalls(ir_entity *method, cpset_t *call_entities, method_env *env)
{
	assert(is_method_entity(method));
	assert(call_entities);
	assert(env);

	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, call_entities);
	ir_entity *call_entity;
	analyzer_env *a_env = env->analyzer_env;
	while ((call_entity = cpset_iterator_next(&iterator)) != NULL) {
		cpmap_t *call_sites = cpmap_find(a_env->dyncall_targets, call_entity);
		assert(call_sites);
		cpset_t *targets = cpmap_find(call_sites, env->entity);
		assert(targets != NULL);
		//assert(cpset_find(targets, method) == NULL); // doesn't make sense!?

		DEBUGOUT(3, "\t\t\t\t\tupdating method %s.%s for call %s.%s\n", get_compound_name(get_entity_owner(method)), get_entity_name(method), get_compound_name(get_entity_owner(call_entity)), get_entity_name(call_entity));
		// add to targets set
		cpset_insert(targets, method);
	}
}

static void update_unused_targets(method_env *env, ir_type *klass)
{
	assert(env);
	assert(klass);

	analyzer_env *a_env = env->analyzer_env;
	cpmap_t *klasses = cpmap_find(a_env->unused_targets, env->entity);
	if (klasses != NULL) {
		cpmap_t *methods = cpmap_find(klasses, klass);
		if (methods != NULL) {
			{
				cpmap_iterator_t iterator;
				cpmap_iterator_init(&iterator, methods);
				cpmap_entry_t entry;
				while ((entry = cpmap_iterator_next(&iterator)).key != NULL || entry.data != NULL) {
					ir_entity *method = (ir_entity *)entry.key;
					cpset_t *call_entities = entry.data;

					add_to_dyncalls(method, call_entities, env);
					if (cpset_size(call_entities) > 0) {
						add_to_workqueue(method, klass, env);
					}
					cpmap_remove_iterator(methods, &iterator);
					cpset_destroy(call_entities);
					free(call_entities);
				}
			}
			cpmap_remove(klasses, klass);
			cpmap_destroy(methods);
			free(methods);
		}
	}
}

static bool is_in_iteration_set(iteration_set *set, void *element)
{
	return cpset_find(set->new, element) != NULL || cpset_find(set->current, element) != NULL
	       || cpset_find(set->propagated, element) != NULL;
}

static void add_new_live_class(ir_type *klass, method_env *env)
{
	assert(is_Class_type(klass));
	assert(env);

	method_info *info = env->info;
	iteration_set *i_set = info->live_classes;
	if (!is_in_iteration_set(i_set, klass) // if it had not already been added
	        && !oo_get_class_is_abstract(klass)) { // if not abstract
		// add to live classes
		cpset_insert(i_set->current, klass);
		DEBUGOUT(3, "\t\t\t\t\tadded new live class %s to method %s.%s \n", get_compound_name(klass), get_compound_name(get_entity_owner(env->entity)), get_entity_name(env->entity));

		// update existing results
		update_unused_targets(env, klass);

	}
}

static void memorize_unused_target(ir_type *klass, ir_entity *entity, ir_entity *call_entity, method_env *env)
{
	assert(is_Class_type(klass));
	assert(is_method_entity(entity));
	assert(is_method_entity(call_entity));
	assert(env);

	analyzer_env *a_env = env->analyzer_env;
	cpmap_t *klasses = cpmap_find(a_env->unused_targets, env->entity);
	if (klasses == NULL) {
		klasses = new_cpmap(hash_ptr, ptr_equals);
		cpmap_set(a_env->unused_targets, env->entity, klasses);
	}
	cpmap_t *methods = cpmap_find(klasses, klass);
	if (methods == NULL) {
		methods = new_cpmap(hash_ptr, ptr_equals);
		cpmap_set(klasses, klass, methods);
	}
	cpset_t *call_entities = cpmap_find(methods, entity);
	if (call_entities == NULL) {
		call_entities = new_cpset(hash_ptr, ptr_equals);
		cpmap_set(methods, entity, call_entities);
	}
	cpset_insert(call_entities, call_entity);
}

static ir_entity *find_entity_by_ldname(ident *ldname)
{
	assert(ldname);

	size_t n = get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		ir_graph *graph = get_irp_irg(i);
		ir_entity *entity = get_irg_entity(graph);
		if (get_entity_ld_ident(entity) == ldname) {
			return entity;
		}
	}

	return NULL;
}

static ir_entity *get_ldname_redirect(ir_entity *entity)
{
	assert(entity);
	assert(is_method_entity(entity));
	assert(get_entity_irg(entity) == NULL);

	ir_entity *target = NULL;

	// external functions like C functions usually have identical name and ldname
	// so assumption is if an method entity without graph, has differing name and ldname, and the ldname belongs to another method with graph, it's a redirection
	const ident *name = get_entity_ident(entity);
	const ident *ldname = get_entity_ld_ident(entity);
	if (name != ldname) {
		target = find_entity_by_ldname(ldname);
	}

	return target;
}

static ir_type *get_class_from_type(ir_type *type);

static void analyzer_handle_no_graph(ir_entity *entity, method_env *env)
{
	assert(entity);
	assert(is_method_entity(entity));
	assert(get_entity_irg(entity) == NULL);
	assert(env);

	DEBUGOUT(4, "\t\t\thandling method without graph %s.%s ( %s )\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));

	// check for redirection to different function via the linker name
	ir_entity *target = get_ldname_redirect(entity);
	if (target != NULL) { // if redirection target exists
		DEBUGOUT(4, "\t\t\t\tentity seems to redirect to different function via the linker name: %s.%s ( %s )\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));
		add_to_workqueue(target, get_entity_owner(entity), env);//Entity owner might not be correct, but it doesn't matter, as live classes are ignored
	}
	else {
		// assume external
		DEBUGOUT(4, "\t\t\tprobably external %s.%s ( %s )\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));

		//Handle external methode (e.g. native)
		//Add all known subclasses of return type live. Hopefully we don't lose much precision.
		method_info *info = env->info;
		iteration_set *i_set = info->live_classes;
		cpset_iterator_t it;
		cpset_iterator_init(&it, info->return_subtypes);
		ir_type *type;
		while ((type = cpset_iterator_next(&it)) != NULL) {
			cpset_insert(i_set->current, type);
		}
	}
}

static inline bool method_as_lib_meth(ir_entity *entity, method_info *info)
{
	return info->is_library_method && !ext_method_is_non_leaking(entity);
}

static void add_to_workqueue(ir_entity *entity, ir_type *klass, method_env *env)
{
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	analyzer_env *a_env = env->analyzer_env;
	method_info *done_entity_info = cpmap_find(a_env->done_map, entity);
	if (done_entity_info == NULL) { // only enqueue if not already done
		DEBUGOUT(3, "\t\t\tadding %s.%s ( %s ) [%s] to workqueue\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity), ((get_entity_irg(entity)) ? "graph" : "nograph"));
		method_env *entity_menv = (method_env *) malloc(sizeof(method_env));
		entity_menv->entity = entity;
		method_info *info = new_method_info();
		iteration_set *i_set = info->live_classes;
		if (is_Class_type(get_entity_owner(entity)) && oo_get_class_is_extern(get_entity_owner(entity))) {
			//Handle external library methods
			info->is_library_method = true;
			DEBUGOUT(5, "%s.%s is marked as library method\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity));
		}
		if(klass != NULL && is_Class_type(klass))
			cpset_insert(i_set->current, klass);//TODO: owner is wrong as we have to add all possible "owners". FIXED now klass
		entity_menv->info = info;
		entity_menv->analyzer_env = a_env;
		cpset_insert(info->callers, env->entity);
		pdeq_putr(a_env->workqueue, entity_menv);
	}
	else {
		cpset_insert(done_entity_info->callers, env->entity);
		if(klass != NULL && is_Class_type(klass)) {
			method_env *entity_menv = malloc(sizeof(*entity_menv));
			entity_menv->entity = entity;
			entity_menv->analyzer_env = a_env;
			entity_menv->info = done_entity_info;
			add_new_live_class(klass, entity_menv);
			free(entity_menv);
		}
		//This is later done for entities added to the workqueue.
		//for already analyzed methods, we update already propagated info here.
		if (entity != env->entity) { //Only if call is non-recursive
			method_info *caller_info = env->info;
			cpset_t cut;
			cpset_init(&cut, hash_ptr, ptr_equals);
			//Parameters
			iteration_set *caller_i_set = caller_info->live_classes;
			iteration_set *callee_i_set = done_entity_info->live_classes;
			cut_sets(&cut, caller_i_set->propagated, done_entity_info->parameter_subtypes);
			if (method_as_lib_meth(entity, done_entity_info)) {
				make_subset(a_env->ext_live_classes, &cut);
			}
			else {
				make_subset(callee_i_set->current, &cut);
			}
			cpset_destroy(&cut);
			cpset_init(&cut, hash_ptr, ptr_equals);
			//Return
			cut_sets(&cut, done_entity_info->return_subtypes, callee_i_set->propagated);
			make_subset(caller_i_set->new, &cut);

			cpset_destroy(&cut);
		}
	}
}

static void take_entity(ir_entity *entity, ir_type *klass, cpset_t *result_set, method_env *env)
{
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	if (cpset_find(result_set, entity) == NULL) { // take each entity only once (the sets won't mind but the workqueue)
		DEBUGOUT(4, "\t\t\ttaking entity %s.%s ( %s ) of %s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity), get_compound_name(klass));


		// add to result set
		cpset_insert(result_set, entity);

		// add to workqueue
		add_to_workqueue(entity, klass, env);
	}
}

static ir_entity *fir_ascend_into_superclasses_and_merge(ir_type *klass, ir_entity *call_entity, ir_entity *current_result); // forward declaration

static ir_entity *find_implementation_recursive(ir_type *klass, ir_entity *call_entity)
{
	assert(klass);
	assert(is_Class_type(klass));
	assert(call_entity);
	assert(is_method_entity(call_entity));

	ir_entity *result = NULL;

	DEBUGOUT(3, "\t\t\t\twalking class %s\n", get_compound_name(klass));

	result = get_class_member_by_name(klass, get_entity_ident(call_entity));

	if (result != NULL) {
		if (oo_get_method_is_abstract(result)) {
			result = NULL;
		}
		else {
			DEBUGOUT(3, "\t\t\t\t\tfound candidate %s.%s ( %s ) [%s]\n", get_compound_name(get_entity_owner(result)), get_entity_name(result), get_entity_ld_name(result), ((get_entity_irg(result)) ? "graph" : "nograph"));
		}
	}
	else {
		result = fir_ascend_into_superclasses_and_merge(klass, call_entity, result);
	}

	return result;
}

static ir_entity *fir_ascend_into_superclasses_and_merge(ir_type *klass, ir_entity *call_entity, ir_entity *current_result)
{
	assert(klass);
	assert(is_Class_type(klass));
	assert(call_entity);
	assert(is_method_entity(call_entity));

	ir_entity *result = current_result;

	size_t n_supertypes = get_class_n_supertypes(klass);
	DEBUGOUT(3, "\t\t\t\t\t%s has %lu superclasses\n", get_compound_name(klass), (unsigned long)n_supertypes);
	for (size_t i = 0; i < n_supertypes; i++) {
		ir_type *superclass = get_class_supertype(klass, i);

		//if (oo_get_class_is_interface(superclass)) continue; // need to ascend in interfaces because of stuff like Java 8 default methods

		ir_entity *r = find_implementation_recursive(superclass, call_entity);

		// merge results
		// more than one is ambiguous, but class methods win against interface default methods (at least in Java 8) !?
		if (result == NULL) {
			result = r;
		}
		else {
			if (r != NULL) {
				ir_type *result_owner          = get_entity_owner(result);
				ir_type *r_owner               = get_entity_owner(r);
				bool     result_from_interface = oo_get_class_is_interface(result_owner);
				bool     r_from_interface      = oo_get_class_is_interface(r_owner);
				bool     r_overrides_result    = is_class_trans_subtype(result_owner, r_owner);
				bool     result_overrides_r    = is_class_trans_subtype(r_owner, result_owner);

				if (result_owner == r_owner) {
					// Nothing to do, we reached the same class by two ways
				}
				else if (result_overrides_r || (r_from_interface && !result_from_interface)) {
					DEBUGOUT(3, "\t\t\t\t\t\tcandidate %s.%s ( %s ) [%s] beats candidate %s.%s ( %s ) [%s]\n",
					         get_compound_name(result_owner), get_entity_name(result), get_entity_ld_name(result), ((get_entity_irg(result)) ? "graph" : "nograph"),
					         get_compound_name(r_owner), get_entity_name(r), get_entity_ld_name(r), ((get_entity_irg(r)) ? "graph" : "nograph"));
				}
				else if (r_overrides_result || (result_from_interface && !r_from_interface)) {
					DEBUGOUT(3, "\t\t\t\t\t\tcandidate %s.%s ( %s ) [%s] beats candidate %s.%s ( %s ) [%s]\n",
					         get_compound_name(r_owner), get_entity_name(r), get_entity_ld_name(r), ((get_entity_irg(r)) ? "graph" : "nograph"),
					         get_compound_name(result_owner), get_entity_name(result), get_entity_ld_name(result), ((get_entity_irg(result)) ? "graph" : "nograph"));
					result = r;
				}
				else if (result_from_interface == r_from_interface) {   // both true or both false
					assert(false && "ambiguous interface implementation");
				}
				else {
					assert(false && "Missing case");
				}
			}
		}
	}

	return result;
}

static ir_entity *find_inherited_implementation(ir_type *klass, ir_entity *call_entity)
{
	assert(klass);
	assert(is_Class_type(klass));
	assert(call_entity);
	assert(is_method_entity(call_entity));
	assert(oo_get_method_is_abstract(call_entity));

	ir_entity *result = NULL;

	result = fir_ascend_into_superclasses_and_merge(klass, call_entity, result);

	return result;
}

static void collect_methods_recursive(ir_entity *call_entity, ir_type *klass, ir_entity *entity, cpset_t *result_set, method_env *env)
{
	assert(call_entity);
	assert(is_method_entity(call_entity));
	assert(klass);
	assert(is_Class_type(klass));
	assert(entity);
	assert(is_method_entity(entity));
	assert(result_set);
	assert(env);

	DEBUGOUT(3, "\t\twalking %s%s %s\n", ((oo_get_class_is_abstract(klass)) ? "abstract " : ""), ((oo_get_class_is_interface(klass)) ? "interface" : "class"), get_compound_name(klass));
	ir_entity *current_entity = entity;

	ir_entity *overwriting_entity = get_class_member_by_name(klass, get_entity_ident(current_entity));
	if (overwriting_entity != NULL && overwriting_entity != current_entity) { // if has overwriting entity
		DEBUGOUT(3, "\t\t\t%s.%s overwrites %s.%s\n", get_compound_name(get_entity_owner(overwriting_entity)), get_entity_name(overwriting_entity), get_compound_name(get_entity_owner(current_entity)), get_entity_name(current_entity));

		current_entity = overwriting_entity;
	}
	// else it is inherited


	// support for FIRM usage without any entity copies at all (not even for case interface method implemention inherited from a superclass) -> have to assume some usual semantics
	// for interface calls (or more general abstract calls) there has to be a non-abstract implementation in each non-abstract subclass, if there is no entity copy we have to find the implementation by ourselves (in cases an inherited method implements the abstract method)
	if (oo_get_method_is_abstract(call_entity) && !oo_get_class_is_abstract(klass) && !oo_get_class_is_interface(klass) && oo_get_method_is_abstract(current_entity)) { // careful: interfaces seem not always to be marked as abstract
		DEBUGOUT(3, "\t\t\tlooking for inherited implementation of abstract method %s.%s\n", get_compound_name(get_entity_owner(call_entity)), get_entity_name(call_entity));
		ir_entity *inherited_impl = find_inherited_implementation(klass, call_entity);
		if (inherited_impl != NULL) {
			DEBUGOUT(3, "\t\t\t\tfound %s.%s as inherited implementation\n", get_compound_name(get_entity_owner(inherited_impl)), get_entity_name(inherited_impl));
			current_entity = inherited_impl;
		}
		else {
			DEBUGOUT(3, "\t\t\t\tfound no inherited implementation to abstract call entity\n");
			//assert(false); // there are problems with X10 structs (they don't have interface implementations because their box classes have them) and with missing entities (e.g. String.ixi in test case ArrayTest)
		}
	}
	method_info *m_info = env->info;
	if (!oo_get_method_is_abstract(current_entity)) { // ignore abstract methods
		if (is_in_iteration_set(m_info->live_classes, klass)) { // if class is considered in use
			take_entity(current_entity, klass, result_set, env);//TODO: Why current_entity and not class
		}
		else {
			DEBUGOUT(3, "\t\t\tclass not in use, memorizing %s.%s %s\n", get_compound_name(get_entity_owner(current_entity)), get_entity_name(current_entity), ((get_entity_irg(current_entity)) ? "G" : "N"));
			memorize_unused_target(klass, current_entity, call_entity, env); // remember entity with this class for patching if this class will become used
		}
	}
	else {
		DEBUGOUT(3, "\t\t\t%s.%s is abstract\n", get_compound_name(get_entity_owner(current_entity)), get_entity_name(current_entity));
	}

	size_t n_subtypes = get_class_n_subtypes(klass);
	DEBUGOUT(3, "\t\t\t%s has %lu subclasses\n", get_compound_name(klass), (unsigned long)n_subtypes);
	for (size_t i = 0; i < n_subtypes; i++) {
		ir_type *subclass = get_class_subtype(klass, i);

		collect_methods_recursive(call_entity, subclass, current_entity, result_set, env);
	}
}

// collect method entities from downwards in the class hierarchy
// it walks down the classes to have the entities with the classes even when the method is inherited
static void collect_methods(ir_entity *call_entity, cpset_t *result_set, method_env *env)
{
	collect_methods_recursive(call_entity, get_entity_owner(call_entity), call_entity, result_set, env);
}

static void analyzer_handle_static_call(ir_node *call, ir_entity *entity, method_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	DEBUGOUT(4, "\tstatic call: %s.%s %s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

	// add to live methods
	//cpset_insert(env->live_methods, entity);

	add_to_workqueue(entity, get_entity_owner(entity), env);


	// hack to detect calls (like class initialization) that are hidden in frontend-specific nodes
	ir_graph *graph = get_entity_irg(entity);
	if (!graph) {
		// ask frontend if there are additional methods called here (e.g. needed to detect class initialization)
		ir_entity *called_method = detect_call(call); //TODO support for more than one
		if (called_method) {
			assert(is_method_entity(called_method));
			//assert(get_entity_irg(called_method)); // can be external
			DEBUGOUT(4, "\t\texternal method calls %s.%s ( %s )\n", get_compound_name(get_entity_owner(called_method)), get_entity_name(called_method), get_entity_ld_name(called_method));
			//cpset_insert(env->live_methods, called_method);
			add_to_workqueue(called_method, get_entity_owner(called_method), env);//Don't know whether owner is always correct
		}
	}
}

static void analyzer_handle_dynamic_call(ir_node *call, ir_entity *entity, method_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	DEBUGOUT(4, "\tdynamic call: %s.%s %s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

	analyzer_env *a_env = env->analyzer_env;
	cpmap_t *call_sites = cpmap_find(a_env->dyncall_targets, entity);
	if (call_sites == NULL) {
		call_sites = new_cpmap(hash_ptr, ptr_equals);
		cpmap_set(a_env->dyncall_targets, entity, call_sites);
	}
	if (cpmap_find(call_sites, env->entity) == NULL) { // if not already done
		// calculate set of all method entities that this call could potentially call

		// first static lookup upwards in the class hierarchy for the case of an inherited method
		// The entity from the MethodSel node is already what the result of a static lookup would be.

		// then collect all potentially called method entities from downwards the class hierarchy
		cpset_t *result_set = new_cpset(hash_ptr, ptr_equals);
		collect_methods(entity, result_set, env);

		// note: cannot check for nonempty result set here because classes could be nonlive at this point but become live later depending on the order in which methods are analyzed
		cpmap_set(call_sites, env->entity, result_set);
	}
}

static cpmap_t known_types;

static void get_all_subtypes(ir_type *klass, cpset_t *res_set)
{
	assert(is_Class_type(klass));
	cpset_insert(res_set, klass);
	size_t subtypes_n = get_class_n_subtypes(klass);
	for (size_t i = 0; i < subtypes_n; i++) {
		get_all_subtypes(get_class_subtype(klass, i), res_set);
	}
}

static cpset_t *get_all_subtypes_c(ir_type *klass)
{
	cpset_t *types = cpmap_find(&known_types, klass);
	if (types == NULL) {
		types = new_cpset(hash_ptr, ptr_equals);
		get_all_subtypes(klass, types);
		cpmap_set(&known_types, klass, types);
	}
	return types;
}

static ir_type *get_class_from_type(ir_type *type)
{
	assert(type);

	if (is_Class_type(type)) {
		DEBUGOUT(3, "class %s\n",  get_compound_name(type));
		return type;
	}
	else if (is_Array_type(type)) {
		DEBUGOUT(3, "array of ");
		return get_class_from_type(get_array_element_type(type));
	}
	else if (is_Pointer_type(type)) {
		DEBUGOUT(3, "pointer to ");
		return get_class_from_type(get_pointer_points_to_type(type));
	}
	else {
		DEBUGOUT(3, "Unsupported or unnecessary type %s found\n", get_type_opcode_name(get_type_opcode(type)));
		return NULL;
	}
}

static cpset_t *get_subclasses_from_type(ir_type *type)
{
	assert(type);
	ir_type *klass = get_class_from_type(type);
	if (klass)
		return get_all_subtypes_c(klass);
	return NULL;
}

static void walk_callgraph_and_analyze(ir_node *node, void *environment)
{
	assert(environment);
	method_env *m_env = (method_env *)environment;
	switch (get_irn_opcode(node)) {
	case iro_Address: {
			ir_node *address = node;
			ir_entity *entity = get_Address_entity(address);
			if (is_method_entity(entity)) {
				// could be a function whose address is taken (although usually the Address node of a normal call, these cases cannot be distinguished)
				DEBUGOUT(4, "\tAddress with method entity: %s.%s %s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));
				DEBUGOUT(4, "\t\tcould be address taken, so it could be called\n");

				// add to live methods
				//cpset_insert(a_env->live_methods, entity);

				//look whether methods calls constructors that aren't in a graph
				analyzer_env *a_env = m_env->analyzer_env;
				cpset_t *classes;
				if ((classes = cpmap_find(a_env->ext_called_constr, entity)) != NULL) {
					cpset_iterator_t iterator;
					cpset_iterator_init(&iterator, classes);
					ir_type *klass;
					while ((klass = cpset_iterator_next(&iterator)) != NULL) {
						add_new_live_class(klass, m_env);
					}
				}
				add_to_workqueue(entity, NULL, m_env);//klass will be added on call, otherwise: does it hurt to pass entity_owner here?
			}
			break;
		}
	case iro_Call: {
			ir_node *call = node;
			ir_node *callee = get_irn_n(call, 1);
			if (is_Address(callee)) {
				// static call
				ir_entity *entity = get_Address_entity(callee);

				analyzer_handle_static_call(call, entity, m_env);

			}
			else if (is_Proj(callee)) {
				ir_node *pred = get_Proj_pred(callee);
				if (is_MethodSel(pred)) {
					ir_node *methodsel = pred;
					ir_entity *entity = get_MethodSel_entity(methodsel);

					if (oo_get_call_is_statically_bound(call)) {
						// weird case of Call with MethodSel that is marked statically bound
						analyzer_handle_static_call(call, entity, m_env);
					}
					else {
						// dynamic call
						analyzer_handle_dynamic_call(call, entity, m_env);
					}
				}
				else {
					// indirect call via function pointers or are there even more types of calls?

					if (get_Tuple_n_preds(pred) == 2 && is_Address(get_Tuple_pred(pred, 1))) {
						ir_entity *entity = get_Address_entity(get_Tuple_pred(pred, 1));

						DEBUGOUT(4, "local devirt static call\n");
						DEBUGOUT(4, "to entity of type%s\n", get_type_opcode_name(get_type_opcode(get_entity_type(entity))));
						if(is_method_entity(entity))
							analyzer_handle_static_call(call, entity, m_env);
					}
					else {
						DEBUGOUT(4, "\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
						DEBUGOUT(4, "-> %s", gdb_node_helper(callee));
						DEBUGOUT(4, "-> %s\n", gdb_node_helper(pred));
					}
				}
			}
			else {
				// indirect call via function pointers or are there even more types of calls?
				DEBUGOUT(4, "\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
				DEBUGOUT(4, "-> %s\n", gdb_node_helper(callee));
			}
			break;
		}
	case iro_Store: {
			ir_node *field = get_irn_n(node, 1);
			if (is_Member(field)) {
				ir_entity *member_entity = get_Member_entity(field);
				if (get_subclasses_from_type(get_entity_type(member_entity)) != NULL) { //Only non primitive type fields are important
					method_info *info = m_env->info;
					cpset_insert(info->field_writes, member_entity);
					DEBUGOUT(4, "\tfield write to: %s\n", get_entity_name(member_entity));
				}
			}
			else if (is_Address(field)) {
				ir_entity *member_entity = get_Address_entity(field);
				if (get_subclasses_from_type(get_entity_type(member_entity)) != NULL) { //Only non primitive type fields are important
					method_info *info = m_env->info;
					cpset_insert(info->field_writes, member_entity);
					DEBUGOUT(4, "\tfield write to: %s\n", get_entity_name(member_entity));
				}
			}
		}
	case iro_Load: {
			ir_node *field = get_irn_n(node, 1);
			if (is_Member(field)) {
				ir_entity *member_entity = get_Member_entity(field);
				if (get_subclasses_from_type(get_entity_type(member_entity)) != NULL) { //Only non primitive type fields are important
					method_info *info = m_env->info;
					cpset_insert(info->field_reads, member_entity);
					DEBUGOUT(4, "\tfield read to: %s\n", get_entity_name(member_entity));
				}
			}
			else if (is_Address(field)) {
				ir_entity *member_entity = get_Address_entity(field);
				if (get_subclasses_from_type(get_entity_type(member_entity)) != NULL) { //Only non primitive type fields are important
					method_info *info = m_env->info;
					cpset_insert(info->field_reads, member_entity);
					DEBUGOUT(4, "\tfield read to: %s\n", get_entity_name(member_entity));
				}
			}
		}
	default:
		if (is_VptrIsSet(node)) {
			// use new VptrIsSet node for detection of object creation
			ir_type *klass = get_VptrIsSet_type(node);
			if (klass != NULL) {
				//printf("%s\n", gdb_node_helper(klass));
				assert(is_Class_type(klass));

				DEBUGOUT(4, "\tVptrIsSet: %s\n", get_compound_name(klass));
				add_new_live_class(klass, m_env);
			}
		}
		// skip other node types
		break;
	}
}

static void debug_print_info(method_info *info)
{
	assert(info);

	printf("===START INFO BLOCK==========\n");
	if (info->is_library_method)
		printf("It's a library method");
	cpset_t *set = info->parameter_subtypes;
	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, set);
	printf("======PARAMETER SUBTYPES=====\n");
	ir_type *type;
	while ((type = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s, ", get_compound_name(type));
	}
	printf("\n");

	set = info->return_subtypes;
	cpset_iterator_init(&iterator, set);
	printf("======RETURN SUBTYPES=====\n");
	while ((type = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s, ", get_compound_name(type));
	}
	printf("\n");

	set = info->field_writes;
	cpset_iterator_init(&iterator, set);
	ir_entity *entity;
	printf("======FIELD WRITES=====\n");
	while ((entity = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s.%s, ", get_compound_name(get_entity_owner(entity)), get_entity_name(entity));
	}
	printf("\n");

	set = info->field_reads;
	cpset_iterator_init(&iterator, set);
	printf("======FIELD READS=====\n");
	while ((entity = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s.%s, ", get_compound_name(get_entity_owner(entity)), get_entity_name(entity));
	}
	printf("\n");

	iteration_set *i_set = info->live_classes;
	printf("======LIVE CLASSES=====\n");
	cpset_iterator_init(&iterator, i_set->propagated);
	printf("p: ");
	while ((type = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s, ", get_compound_name(type));
	}
	cpset_iterator_init(&iterator, i_set->current);
	printf("c: ");
	while ((type = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s, ", get_compound_name(type));
	}
	cpset_iterator_init(&iterator, i_set->new);
	printf("n: ");
	while ((type = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s, ", get_compound_name(type));
	}
	printf("\n");

	set = info->callers;
	cpset_iterator_init(&iterator, set);
	printf("======CALLERS=====\n");
	while ((entity = cpset_iterator_next(&iterator)) != NULL) {
		printf("%s.%s, ", get_compound_name(get_entity_owner(entity)), get_entity_name(entity));
	}
	printf("\n");
	printf("===END INFO BLOCK=========\n");
}

static bool make_subset_r(cpset_t *a, cpset_t *b)
{
	assert(a);
	assert(b);

	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, b);
	void  *element;
	bool change = false;
	while ((element = cpset_iterator_next(&iterator)) != NULL) {
		change = change || !cpset_find(a, element);
		cpset_insert(a, element);
	}
	return change;
}

/**
 * Make b subset of a
 **/
static void make_subset(cpset_t *a, cpset_t *b)
{
	assert(a);
	assert(b);

	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, b);
	void  *element;
	while ((element = cpset_iterator_next(&iterator)) != NULL) {
		cpset_insert(a, element);
	}
}

static void collect_arg_and_res_types(method_env *env)
{
	assert(env);

	method_info *info = env->info;
	//Arguments
	ir_type *entity_type = get_entity_type(env->entity);
	size_t arg_n = get_method_n_params(entity_type);
	size_t i = 1;
	if (method_as_lib_meth(env->entity, info) || is_constr(env->entity)) {
		i = 0; //external methods get this pointer too
	}
	for (; i < arg_n; i++) {
		ir_type *param_type = get_method_param_type(entity_type, i);
		DEBUGOUT(3, "Getting parameter ");
		cpset_t *subtypes = get_subclasses_from_type(param_type);
		if (subtypes != NULL)
			make_subset(info->parameter_subtypes, subtypes);
	}
	//Result
	size_t res_n = get_method_n_ress(entity_type);
	for (size_t i = 0; i < res_n; i++) {
		ir_type *res_type = get_method_res_type(entity_type, i);
		DEBUGOUT(3, "Getting result ");
		cpset_t *subtypes = get_subclasses_from_type(res_type);
		if (subtypes != NULL)
			make_subset(info->return_subtypes, subtypes);
	}

}

static inline bool field_as_external(ir_entity *field)
{
	return (is_Class_type(get_entity_owner(field))
	        && oo_get_class_is_extern(get_entity_owner(field)))
	       || get_entity_owner(field) == get_glob_type();
}

static bool add_live_classes_to_field(analyzer_env *env, ir_entity *field, cpset_t *klasses)
{
	assert(env);
	assert(field);
	assert(klasses);

	cpset_t *field_set;
	if (field_as_external(field)) {
		field_set = env->ext_live_classes;
	}
	else {
		iteration_set *field_i_set = cpmap_find(env->live_classes_fields, field);
		if (field_i_set == NULL) {
			field_i_set = new_iteration_set();
			cpmap_set(env->live_classes_fields, field, field_i_set);
		}
		field_set = field_i_set->new;
	}
	return make_subset_r(field_set, klasses);
}

static void cut_sets(cpset_t *res, cpset_t *a, cpset_t *b)
{
	assert(res);
	assert(a);
	assert(b);

	cpset_t *c = b;
	if (cpset_size(a) > cpset_size(b)) {
		c = a;
		a = b;
	}
	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, a);
	void *element;
	while ((element = cpset_iterator_next(&iterator)) != NULL) {
		if (cpset_find(c, element))
			cpset_insert(res, element);
	}
}

static void handle_new_classes_in_current_set(iteration_set *i_set, method_env *env)
{
	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, i_set->current);
	ir_type  *element;
	while ((element = cpset_iterator_next(&iterator)) != NULL) {
		if (cpset_find(i_set->propagated, element)) {
			cpset_remove_iterator(i_set->current, &iterator);
		}
		else {
			update_unused_targets(env, element);
		}
	}
}

static bool update_iteration_sets(analyzer_env *env)
{
	cpmap_iterator_t iterator;
	cpmap_iterator_init(&iterator, env->live_classes_fields);
	cpmap_entry_t entry;
	bool change = false;
	iteration_set *i_set;
	cpset_t *old;
	while ((entry = cpmap_iterator_next(&iterator)).data != NULL) {
		i_set = entry.data;
		make_subset(i_set->propagated, i_set->current);
		cpset_destroy(i_set->current);
		cpset_init(i_set->current, hash_ptr, ptr_equals);
		old = i_set->current;
		i_set->current = i_set->new;
		i_set->new = old;
		change = change || cpset_size(i_set->current) != 0;
	}
	cpmap_iterator_init(&iterator, env->done_map);
	method_env m_env;
	m_env.analyzer_env = env;
	ir_entity *entity;
	method_info *info;
	while ((entry = cpmap_iterator_next(&iterator)).data != NULL) {
		info = entry.data;
		entity = (ir_entity *) entry.key;
		m_env.entity = entity;
		m_env.info = info;
		i_set = info->live_classes;
		make_subset(i_set->propagated, i_set->current);
		cpset_destroy(i_set->current);
		cpset_init(i_set->current, hash_ptr, ptr_equals);
		old = i_set->current;
		i_set->current = i_set->new;
		i_set->new = old;
		handle_new_classes_in_current_set(i_set, &m_env);
		change = change || cpset_size(i_set->current) != 0;
	}
	return change;
}

static bool propagate_live_classes(analyzer_env *env)
{
	assert(env);
	cpmap_t *done_map = env->done_map;
	cpmap_iterator_t iterator;
	cpmap_iterator_init(&iterator, done_map);
	cpmap_entry_t entry;
	bool change = false;
	while ((entry = cpmap_iterator_next(&iterator)).key != NULL || entry.data != NULL) {
		method_info *info = (method_info *) entry.data;
		ir_entity *callee = (ir_entity *) entry.key;
		iteration_set *callee_i_set = info->live_classes;
		cpset_iterator_t set_iterator;
		//Method calls
		cpset_iterator_init(&set_iterator, info->callers);
		ir_entity *caller;
		while ((caller = cpset_iterator_next(&set_iterator)) != NULL) {
			method_info *caller_info = cpmap_find(done_map, caller);
			cpset_t cut;
			cpset_init(&cut, hash_ptr, ptr_equals);
			//Parameters
			iteration_set *caller_i_set = caller_info->live_classes;
			if (cpset_size(caller_i_set->current) != 0) {
				cut_sets(&cut, caller_i_set->current, info->parameter_subtypes);
				if (method_as_lib_meth(callee, info)) {
					make_subset(env->ext_live_classes, &cut);
				}
				else {
					make_subset(callee_i_set->new, &cut);
				}
			}
			cpset_destroy(&cut);
			cpset_init(&cut, hash_ptr, ptr_equals);
			//Return
			if (cpset_size(callee_i_set->current) != 0) {
				cut_sets(&cut, info->return_subtypes, callee_i_set->current);
				make_subset(caller_i_set->new, &cut);
			}

			cpset_destroy(&cut);
		}
		//Field Reads
		cpset_iterator_init(&set_iterator, info->field_reads);
		ir_entity *field;
		while ((field = cpset_iterator_next(&set_iterator)) != NULL) {
			if (field_as_external(field)) {
				cpset_t cut;
				cpset_init(&cut, hash_ptr, ptr_equals);
				cut_sets(&cut, env->ext_live_classes, get_subclasses_from_type(get_entity_type(field)));
				make_subset(callee_i_set->new, &cut);
				cpset_destroy(&cut);
			}
			else {
				iteration_set *field_i_set = cpmap_find(env->live_classes_fields, field);
				if (field_i_set && cpset_size(field_i_set->current) != 0) {
					make_subset(callee_i_set->new, field_i_set->current);
				}
			}
		}
		//Field Writes
		cpset_iterator_init(&set_iterator, info->field_writes);
		while ((field = cpset_iterator_next(&set_iterator)) != NULL) {
			cpset_t *subtypes;
			//DEBUGOUT("Field type of %s.%s is %s", get_compound_name(get_entity_owner(field)), get_entity_name(field), get_type_opcode_name(get_type_opcode(get_entity_type(field))));
			subtypes = get_subclasses_from_type(get_entity_type(field));
			cpset_t cut;
			cpset_init(&cut, hash_ptr, ptr_equals);
			if (cpset_size(callee_i_set->current) != 0) {
				cut_sets(&cut, subtypes, callee_i_set->current);
				bool res = add_live_classes_to_field(env, field, &cut);
				change = change || res;


#if (DEBUG_XTA > 3)
				if (res) {
					if(!field_as_external(field)) {
						printf("Field %s.%s updated to:\n", get_compound_name(get_entity_owner(field)), get_entity_name(field));
						iteration_set *i_set = cpmap_find(env->live_classes_fields, field);
						cpset_iterator_t iterator;
						cpset_iterator_init(&iterator, i_set->propagated);
						printf("p: ");
						ir_type *type;
						while ((type = cpset_iterator_next(&iterator)) != NULL) {
							printf("%s, ", get_compound_name(type));
						}
						cpset_iterator_init(&iterator, i_set->current);
						printf("c: ");
						while ((type = cpset_iterator_next(&iterator)) != NULL) {
							printf("%s, ", get_compound_name(type));
						}
						cpset_iterator_init(&iterator, i_set->new);
						printf("n: ");
						while ((type = cpset_iterator_next(&iterator)) != NULL) {
							printf("%s, ", get_compound_name(type));
						}
						printf("\n");
					} else {
						printf("(External Field)\n");
					}
				}
#endif
			}
			cpset_destroy(&cut);
		}
	}

	//Handle externally callable methods
	cpset_iterator_t it;
	cpset_iterator_init(&it, env->methods_ext_called);
	ir_entity *ec_method; //externally callable method
	while ((ec_method = cpset_iterator_next(&it)) != NULL) {
		if (cpset_find(env->ext_live_classes, get_entity_owner(ec_method))) {
			method_info *info = cpmap_find(env->done_map, ec_method);
			iteration_set *i_set = info->live_classes;
			cpset_t cut;
			cpset_init(&cut, hash_ptr, ptr_equals);
			//Parameters
			cut_sets(&cut, env->ext_live_classes, info->parameter_subtypes);
			make_subset(i_set->new, &cut);
			cpset_destroy(&cut);
			cpset_init(&cut, hash_ptr, ptr_equals);
			//Return
			if (cpset_size(i_set->current) != 0) {
				cut_sets(&cut, info->return_subtypes, i_set->current);
				make_subset(env->ext_live_classes, &cut);
			}

			cpset_destroy(&cut);
		}
	}
	return update_iteration_sets(env) || change;
}

static void del_done_map(cpmap_t *done_map)
{
	assert(done_map);

	cpmap_iterator_t iterator;
	cpmap_iterator_init(&iterator, done_map);
	cpmap_entry_t entry;
	while ((entry = cpmap_iterator_next(&iterator)).data != NULL) {
		method_info *info = (method_info *) entry.data;
		cpset_destroy(info->field_writes);
		cpset_destroy(info->field_reads);
		cpset_destroy(info->parameter_subtypes);
		iteration_set *i_set = info->live_classes;
		cpset_destroy(i_set->new);
		cpset_destroy(i_set->propagated);
		cpset_destroy(i_set->current);
		free(i_set->propagated);
		free(i_set->current);
		free(i_set->new);
		free(i_set);
		cpset_destroy(info->callers);
		free(info->field_writes);
		free(info->field_reads);
		free(info->parameter_subtypes);
		free(info->callers);
		free(info);
	}
	cpmap_destroy(done_map);
}

static void del_live_classes_fields(cpmap_t *live_classes_fields)
{
	assert(live_classes_fields);

	cpmap_iterator_t iterator;
	cpmap_iterator_init(&iterator, live_classes_fields);
	cpmap_entry_t entry;
	while ((entry = cpmap_iterator_next(&iterator)).data != NULL) {
		iteration_set *i_set = entry.data;
		cpset_destroy(i_set->new);
		cpset_destroy(i_set->propagated);
		cpset_destroy(i_set->current);
		free(i_set->current);
		free(i_set->new);
		free(i_set->propagated);
		free(i_set);
	}
	cpmap_destroy(live_classes_fields);
}

static void retrieve_methods_overwriting_external_recursive(analyzer_env *env, ir_type *type, cpset_t *methods, cpset_t *done_types)
{
	for (size_t i = 0, n = get_class_n_members(type); i < n; ++i) {
		ir_entity *entity = get_class_member(type, i);
		if (is_method_entity(entity)) {
			char *id = (char *) get_entity_ident(entity);
			if (oo_get_class_is_extern(type)) {
				//Consider all methods from external methods on the path
				//Implicityly: all external classes are above all app classes in hierachy
				cpset_insert(methods, id);
			}
			else {
				if (cpset_find(methods, id) && !is_constr(entity)) {
					cpset_insert(env->methods_ext_called, entity);
					method_env *meth_env = (method_env *) malloc(sizeof(method_env));
					meth_env->entity = entity;
					meth_env->analyzer_env = env;
					method_info *info = new_method_info();
					iteration_set *i_set = info->live_classes;
					cpset_insert(i_set->current, get_entity_owner(entity));
					meth_env->info = info;
					pdeq_putr(env->workqueue, meth_env);
					//printf("METHOD EXTERNALLY CALLABLE: %s.%s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity));
				}
			}
		}
	}

	for (size_t i = 0, n = get_class_n_subtypes(type); i < n; ++i) {
		ir_type *subtype = get_class_subtype(type, i);
		if (!cpset_find(done_types, subtype)) {
			cpset_insert(done_types, subtype);
			retrieve_methods_overwriting_external_recursive(env, subtype, methods, done_types);
		}
	}
}

static void retrieve_library_info(analyzer_env *env)
{
	for (size_t i = 0, n = get_irp_n_types(); i < n; ++i) {
		ir_type *type = get_irp_type(i);
		if (is_Class_type(type) && oo_get_class_is_extern(type))
			cpset_insert(env->ext_live_classes, type);
		//else if(is_Class_type(type))
		//printf("Type found: %s\n", get_compound_name(type));
	}
	cpset_iterator_t iterator;
	cpset_iterator_init(&iterator, env->ext_live_classes);
	ir_type *type;
	cpset_t methods;
	cpset_t done_types;
	cpset_init(&done_types, hash_ptr, ptr_equals);
	while ((type = cpset_iterator_next(&iterator)) != NULL) {
		//printf("EXTERNAL CLASS: %s\n", get_compound_name(type));
		cpset_init(&methods, hash_ptr, ptr_equals);
		if (!cpset_find(&done_types, type)) {
			retrieve_methods_overwriting_external_recursive(env, type, &methods, &done_types);
			cpset_insert(&done_types, type);
		}
		cpset_destroy(&methods);
	}
	cpset_destroy(&done_types);
}

#ifdef XTA_STATS
static size_t n_fields = 0;
static size_t n_ext_fields = 0;
#endif

/** run Rapid Type Analysis
 * It runs over a reduced callgraph and detects which classes and methods are actually used and computes reduced sets of potentially called targets for each dynamically bound call.
 * @note See the important notes in the documentation of function rta_optimization in the header file!
 * @param entry_points NULL-terminated array of method entities, give all entry points to program code, may _not_ be NULL and must contain at least one method entity, also all entry points should have a graph
 * @param initial_live_classes NULL-terminated array of classes that should always be considered live, may be NULL
 * @param live_classes give pointer to empty uninitialized set for receiving results, This is where all live classes are put (as ir_type*).
 * @param live_methods give pointer to empty uninitialized set for receiving results, This is where all live methods are put (as ir_entity*).
 * @param dyncall_targets give pointer to empty uninitialized map for receiving results, This is where call entities are mapped to their actually used potential call targets (ir_entity* -> {ir_entity*}). It's used to optimize dynamically bound calls if possible. (see also function rta_optimize_dyncalls)
 */
static void xta_run(ir_entity **entry_points, ir_type **initial_live_classes, cpmap_t *dyncall_targets, cpmap_t *ext_called_constr)
{
	assert(entry_points);
	assert(dyncall_targets);

	cpmap_t live_classes_fields;
	cpmap_init(&live_classes_fields, hash_ptr, ptr_equals);
	cpmap_init(dyncall_targets, hash_ptr, ptr_equals);

	cpmap_t unused_targets;
	cpmap_init(&unused_targets, hash_ptr, ptr_equals);

	pdeq *workqueue = new_pdeq();

	cpmap_t done_map;
	cpmap_init(&done_map, hash_ptr, ptr_equals);

	cpmap_init(&known_types, hash_ptr, ptr_equals);

	cpset_t ext_live_classes;
	cpset_init(&ext_live_classes, hash_ptr, ptr_equals);

	analyzer_env env = {
		.workqueue = workqueue,
		.done_map = &done_map,
		.live_classes_fields = &live_classes_fields,
		.dyncall_targets = dyncall_targets,
		.unused_targets = &unused_targets,
		.ext_called_constr = ext_called_constr,
		.ext_live_classes = &ext_live_classes,
		.methods_ext_called = new_cpset(hash_ptr, ptr_equals),
	};

	retrieve_library_info(&env);

	compute_inh_transitive_closure();


	{
		// add all given entry points to live methods and to workqueue
		size_t i = 0;
		DEBUGOUT(2, "entrypoints:\n");
		ir_entity *entity;
		for (; (entity = entry_points[i]) != NULL; i++) {
			assert(is_method_entity(entity));
			//	cpset_insert(live_methods, entity);
			method_info *info = new_method_info();
			// add to workqueue
			ir_graph *graph = get_entity_irg(entity);
			assert(graph); // don't give methods without a graph as entry points for the analysis
			method_env *meth_env = (method_env *) malloc(sizeof(method_env));
			meth_env->entity = entity;
			meth_env->analyzer_env = &env;
			meth_env->info = info;

			pdeq_putr(workqueue, meth_env);

			iteration_set *i_set = info->live_classes;
			//add all subtypes of the parameter to the live set
			collect_arg_and_res_types(meth_env);
			cpset_iterator_t it;
			cpset_iterator_init(&it, info->parameter_subtypes);
			ir_type *klass;
			while ((klass = cpset_iterator_next(&it)) != NULL)
				cpset_insert(i_set->current, klass);

		}
		assert(i > 0 && "give at least one entry point");
	}
	do {
		DEBUGOUT(5, "PROPAGATION FINISHED\n");
		while (!pdeq_empty(workqueue)) {
			method_env *m_env = pdeq_getl(workqueue);
			assert(m_env && is_method_entity(m_env->entity));
			ir_entity *entity = m_env->entity;
			method_info *info = m_env->info;
			method_info *done_info = cpmap_find(&done_map, entity);
			if ( done_info != NULL) { //Handle case where method got queued twice
				cpset_iterator_t it;
				cpset_iterator_init(&it, info->callers);
				ir_entity *caller;
				while ((caller = cpset_iterator_next(&it)) != NULL) {
					method_env caller_env;
					caller_env.analyzer_env = &env;
					caller_env.entity = caller;
					caller_env.info = cpmap_find(&done_map, caller);
					if (caller_env.info == NULL)
						DEBUGOUT(5, "CALLER IS NULL");
					add_to_workqueue(entity, NULL, &caller_env);//NULL here because in the loop below we add all live classes manually
				}
				cpset_iterator_init(&it, info->live_classes->current);
				ir_type *klass;
				m_env->info = done_info;
				while((klass = cpset_iterator_next(&it)) != NULL) {//There should only be on class in it
					add_new_live_class(klass, m_env);
				}
				continue;
			}

			iteration_set *callee_i_set = info->live_classes;
			// add all given initial live classes to live classes
			if (initial_live_classes != NULL) {
				DEBUGOUT(3, "\ninitial live classes of %s:\n", get_entity_name(entity));
				ir_type *klass;
				for (size_t i = 0; (klass = initial_live_classes[i]) != NULL; i++) {
					assert(is_Class_type(klass));
					DEBUGOUT(3, "\t%s\n", get_compound_name(klass));
					cpset_insert(callee_i_set->current, klass);
				}
			}

			DEBUGOUT(1, "== %s.%s (%s)\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));
			cpmap_set(&done_map, entity, m_env->info); // mark as done _before_ walking to not add it again in case of recursive calls
			DEBUGOUT(4, "COLLECTING ARGS AND RES TYPES\n");
			collect_arg_and_res_types(m_env);
			DEBUGOUT(4, "COLLECTING LIVE CLASSES FROM CALLLERS\n");
			//This has to be done here, because already propagated classes will not be looked at during propagaton.
			cpset_iterator_t set_iterator;
			//Method calls
			cpset_iterator_init(&set_iterator, info->callers);
			ir_entity *caller;
			while ((caller = cpset_iterator_next(&set_iterator)) != NULL) {
				method_info *caller_info = cpmap_find(&done_map, caller);
				cpset_t cut;
				cpset_init(&cut, hash_ptr, ptr_equals);
				//Parameters
				iteration_set *caller_i_set = caller_info->live_classes;
				cut_sets(&cut, caller_i_set->propagated, info->parameter_subtypes);
				if (method_as_lib_meth(entity, info)) {
					make_subset(env.ext_live_classes, &cut);
				}
				else {
					make_subset(callee_i_set->current, &cut);
				}

				cpset_destroy(&cut);
			}

			DEBUGOUT(4, "CURRENT INFO\n");
			DEBUGCALL(4) debug_print_info(m_env->info);

			DEBUGOUT(4, "ANALYZING GRAPH\n");
			ir_graph *graph = get_entity_irg(entity);
			if (graph == NULL) {
				analyzer_handle_no_graph(entity, m_env);
			}
			else {
				// analyze graph
				irg_walk_graph(graph, NULL, walk_callgraph_and_analyze, m_env);
			}

			//Handle already propagated live classes of fields, that are read from (Therefore doing it after analyzing graph)
			//Field Reads
			cpset_iterator_init(&set_iterator, info->field_reads);
			ir_entity *field;
			while ((field = cpset_iterator_next(&set_iterator)) != NULL) {
				if (field_as_external(field)) {
					cpset_t cut;
					cpset_init(&cut, hash_ptr, ptr_equals);
					cut_sets(&cut, env.ext_live_classes, get_subclasses_from_type(get_entity_type(field)));
					make_subset(callee_i_set->current, &cut);
					cpset_destroy(&cut);
				}
				else {
					iteration_set *field_i_set = cpmap_find(env.live_classes_fields, field);
					if (field_i_set) {
						make_subset(callee_i_set->current, field_i_set->propagated);
					}
				}
			}

			free(m_env);
		}
	}
	while (propagate_live_classes(&env));

	if (DEBUG_XTA) {
		DEBUGOUT(2, "\n\n==== Results ==============================================\n");
		{
			cpmap_iterator_t iterator;
			cpmap_iterator_init(&iterator, &done_map);
			cpmap_entry_t entry;
			DEBUGOUT(2, "FINAL INFO\n");
			while ((entry = cpmap_iterator_next(&iterator)).key != NULL || entry.data != NULL) {
				const ir_entity *entity = entry.key;
				method_info *info = entry.data;
				DEBUGOUT(2, "\n== %s.%s ( %s )\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));
				DEBUGCALL(2) debug_print_info(info);
			}

		}
		{
			DEBUGOUT(2, "\ndyncall target sets (%lu):\n", (unsigned long)cpmap_size(dyncall_targets));
			//DEBUGOUT("size %u\n", cpmap_size(dyncall_targets));
			cpmap_iterator_t iterator;
			cpmap_iterator_init(&iterator, dyncall_targets);
			cpmap_entry_t entry;
			while ((entry = cpmap_iterator_next(&iterator)).key != NULL || entry.data != NULL) {
				const ir_entity *call_entity = entry.key;
				assert(call_entity);
				DEBUGOUT(2, "\t%s.%s %s\n", get_compound_name(get_entity_owner(call_entity)), get_entity_name(call_entity), (oo_get_class_is_extern(get_entity_owner(call_entity))) ? "external" : "");

				cpmap_t *call_sites = entry.data;
				assert(call_sites);
				DEBUGOUT(2, "\tcall sites (%lu):\n", (unsigned long)cpmap_size(call_sites));
				cpmap_iterator_t iter;
				cpmap_iterator_init(&iter, call_sites);
				cpmap_entry_t e;
				while ((e = cpmap_iterator_next(&iter)).key != NULL || e.data != NULL) {
					const ir_entity *call_site = e.key;
					assert(call_site);
					DEBUGOUT(2, "\t\t%s.%s\n", get_compound_name(get_entity_owner(call_site)), get_entity_name(call_site));

					cpset_t *targets = e.data;
					assert(targets);
					cpset_iterator_t it;
					cpset_iterator_init(&it, targets);
					ir_entity *method;
					while ((method = cpset_iterator_next(&it)) != NULL) {
						//DEBUGOUT("\t\t%s.%s %s\n", get_compound_name(get_entity_owner(method)), get_entity_name(method), gdb_node_helper(method));
						DEBUGOUT(2, "\t\t\t%s.%s %s\n", get_compound_name(get_entity_owner(method)), get_entity_name(method), (oo_get_class_is_extern(get_entity_owner(call_entity))) ? "external" : "");
					}
				}
			}
		}
		DEBUGOUT(2, "\n=============================================================\n");
	}

#ifdef XTA_STATS
	{
		cpmap_iterator_t it;
		cpmap_iterator_init(&it, env.live_classes_fields);

		cpmap_entry_t entry;
		while ((entry = cpmap_iterator_next(&it)).key != NULL) {
			ir_entity *entity = (ir_entity *) entry.key;
			if (is_Class_type(get_entity_owner(entity)) && oo_get_class_is_extern(get_entity_owner(entity)))
				n_ext_fields++;
			else
				n_fields++;
		}
	}
#endif



	// free data structures
	del_pdeq(workqueue);
	del_done_map(&done_map);
	del_live_classes_fields(&live_classes_fields);

	{
		/*cpset_destroy(ext_live_classes->new);
		cpset_destroy(ext_live_classes->propagated);
		cpset_destroy(ext_live_classes->current);
		free(ext_live_classes->current);
		free(ext_live_classes->new);
		free(ext_live_classes->propagated);
		free(ext_live_classes);*/
		cpset_destroy(env.ext_live_classes);

		cpset_destroy(env.methods_ext_called);
		free(env.methods_ext_called);

	}

	{
		// delete the maps and sets in map unused_targets
		cpmap_iterator_t iterator;
		cpmap_iterator_init(&iterator, &unused_targets);
		cpmap_entry_t entry;
		while ((entry = cpmap_iterator_next(&iterator)).key != NULL || entry.data != NULL) {
			cpmap_t *map = entry.data;
			assert(map);

			cpmap_iterator_t inner_iterator;
			cpmap_iterator_init(&inner_iterator, map);
			cpmap_entry_t inner_entry;
			while ((inner_entry = cpmap_iterator_next(&inner_iterator)).key != NULL || inner_entry.data != NULL) {
				cpmap_t *inner_data = inner_entry.data;
				assert(inner_data);

				cpmap_iterator_t innerst_iterator;
				cpmap_iterator_init(&innerst_iterator, inner_data);
				cpmap_entry_t innerst_entry;
				while ((innerst_entry = cpmap_iterator_next(&innerst_iterator)).key != NULL || innerst_entry.data != NULL) {
					cpset_t *set = innerst_entry.data;
					assert(set);

					cpmap_remove_iterator(inner_data, &innerst_iterator);
					cpset_destroy(set);
					free(set);
				}
				cpmap_remove_iterator(map, &inner_iterator);
				cpmap_destroy(inner_data);
				free(inner_data);
			}
			cpmap_remove_iterator(&unused_targets, &iterator);
			cpmap_destroy(map);
			free(map);
		}
	}
	cpmap_destroy(&unused_targets);

	// note: live_classes, live_methods and dyncall_targets are given from outside and return the results, but the sets in map dyncall_targets are allocated in the process and have to be deleted later

	{
		// delete the map and sets in map known_types
		cpmap_iterator_t iterator;
		cpmap_iterator_init(&iterator, &known_types);
		cpmap_entry_t entry;
		while ((entry = cpmap_iterator_next(&iterator)).data != NULL) {
			cpset_t *set = entry.data;
			cpset_destroy(set);
		}
		cpmap_destroy(&known_types);
	}
}


/** frees memory allocated for the results returned by function run_rta
 * @note does not free the memory of the sets and maps themselves, just their content allocated during XTA
 * @param live_classes as returned by rta_run
 * @param live_methods as returned by rta_run
 * @param dyncall_targets as returned by rta_run
 */
static void xta_dispose_results(cpmap_t *dyncall_targets)
{
	assert(dyncall_targets);

	cpmap_iterator_t it;
	cpmap_entry_t entry;
	// delete the sets in map dyncall_targets
	cpmap_iterator_init(&it, dyncall_targets);
	while ((entry = cpmap_iterator_next(&it)).key != NULL || entry.data != NULL) {
		cpmap_t *map = entry.data;
		cpmap_iterator_t iterator;
		cpmap_entry_t e;
		cpmap_iterator_init(&iterator, map);

		while ((e = cpmap_iterator_next(&iterator)).key != NULL || e.data != NULL) {
			cpset_t *set = e.data;
			cpmap_remove_iterator(map, &iterator);
			assert(set);
			cpset_destroy(set);
			free(set);
		}
		cpmap_remove_iterator(dyncall_targets, &it);
		cpmap_destroy(map);
		free(map);
	}
	cpmap_destroy(dyncall_targets);
}


typedef struct optimizer_env {
	pdeq *workqueue; // workqueue for the run over the (reduced) callgraph
	cpset_t *done_set; // set to mark graphs that were already analyzed
	cpmap_t *dyncall_targets; // map that stores the set of potential call targets for every method entity appearing in a dynamically bound call (Map: call entity -> Set: method entities)
#ifdef XTA_STATS
	unsigned long long n_staticcalls; // number of static calls
	unsigned long long n_dyncalls; // number of dynamic calls (without interface calls)
	unsigned long long n_icalls; // number of interface calls
	unsigned long long n_devirts; // number of devirtualizations of dynamic calls (without interface calls)
	unsigned long long n_devirts_icalls; // number of devirtualizations of interface calls
	unsigned long long n_others; // number of other calls (e.g. indirect calls)
	unsigned long long n_reachmeth;
#endif
} optimizer_env;

static void optimizer_add_to_workqueue(ir_entity *method, optimizer_env *env)
{
	assert(is_method_entity(method));
	assert(env);

	if (cpset_find(env->done_set, method) == NULL) { // only enqueue if not already done
		DEBUGOUT(4, "\t\tadding %s.%s to workqueue\n", get_compound_name(get_entity_owner(method)), get_entity_name(method));
		pdeq_putr(env->workqueue, method);
	}
}

static void optimizer_handle_no_graph(ir_entity *entity, optimizer_env *env)
{
	assert(entity);
	assert(is_method_entity(entity));
	assert(get_entity_irg(entity) == NULL);
	assert(env);

	DEBUGOUT(4, "\t\t\thandling method without graph %s.%s ( %s )\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));

	// check for redirection to different function via the linker name
	ir_entity *target = get_ldname_redirect(entity);
	if (target != NULL) { // if redirection target exists
		DEBUGOUT(4, "\t\t\t\tentity seems to redirect to different function via the linker name: %s.%s ( %s )\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));
		optimizer_add_to_workqueue(target, env);
		return; // don't do anything else afterwards in this function
	}

	// currently nothing else to do
}

static void optimizer_handle_static_call(ir_node *call, ir_entity *entity, optimizer_env *env)
{
	assert(call);
	assert(is_Call(call));
	assert(entity);
	assert(is_method_entity(entity));
	assert(env);

	DEBUGOUT(4, "\tstatic call: %s.%s %s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), gdb_node_helper(entity));

#ifdef XTA_STATS
	env->n_staticcalls++;
#endif

	optimizer_add_to_workqueue(entity, env);


	// hack to detect calls (like class initialization) that are hidden in frontend-specific nodes
	ir_graph *graph = get_entity_irg(entity);
	if (!graph) {
		ir_entity *called_method = detect_call(call);
		if (called_method) {
			assert(is_method_entity(called_method));
			//assert(get_entity_irg(called_method)); // can be external
			DEBUGOUT(4, "\t\texternal method calls %s.%s (%s)\n", get_compound_name(get_entity_owner(called_method)), get_entity_name(called_method), get_entity_ld_name(called_method));
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
	DEBUGOUT(4, "\tdynamic call: %s.%s %s\n", get_compound_name(owner), get_entity_name(entity), gdb_node_helper(entity));

#ifdef XTA_STATS
	if (oo_get_class_is_interface(owner))
		env->n_icalls++;
	else
		env->n_dyncalls++;
#endif


	cpmap_t *call_sites = cpmap_find(env->dyncall_targets, entity);
	assert(call_sites);
	cpset_t *targets = cpmap_find(call_sites, get_irg_entity(get_irn_irg(call))); //TODO: Implement a nicer solution
	assert(targets);
	// note: cannot check for nonempty target set here because there can be legal programs that have calls with empty target sets although they will probably run into an exception when executed! (e.g. interface call without implementing class and program initializes reference to null, actually same with abstract class or nonlive class)

	if (cpset_size(targets) == 1 && (!oo_get_class_is_extern(owner) || oo_get_class_is_final(owner) || oo_get_method_is_final(entity))) { // exactly one target and not extern nonfinal
		// devirtualize call
		cpset_iterator_t it;
		cpset_iterator_init(&it, targets);
		ir_entity *target = cpset_iterator_next(&it);
		assert(cpset_iterator_next(&it) == NULL);

#ifdef XTA_STATS
		if (oo_get_class_is_interface(owner))
			env->n_devirts_icalls++;
		else
			env->n_devirts++;
#endif

		// set an Address node as callee to make the call statically bound
		DEBUGOUT(2, "\t\tdevirtualizing call %s.%s -> %s.%s\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_compound_name(get_entity_owner(target)), get_entity_name(target));
		DEBUGOUT(2, "\t\tin method %s.%s\n", get_compound_name(get_entity_owner(get_irg_entity(get_irn_irg(call)))), get_entity_name(get_irg_entity(get_irn_irg(call))));
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

static void walk_callgraph_and_devirtualize(ir_node *node, void *environment)
{
	assert(environment);
	optimizer_env *env = (optimizer_env *)environment;

	switch (get_irn_opcode(node)) {
	case iro_Call: {
			ir_node *call = node;
			ir_node *callee = get_irn_n(call, 1);
			if (is_Address(callee)) {
				// static call
				ir_entity *entity = get_Address_entity(callee);

				optimizer_handle_static_call(call, entity, env);

			}
			else if (is_Proj(callee)) {
				ir_node *pred = get_Proj_pred(callee);
				if (is_MethodSel(pred)) {
					ir_node *methodsel = pred;
					ir_entity *entity = get_MethodSel_entity(methodsel);

					if (oo_get_call_is_statically_bound(call)) {
						// weird case of Call with MethodSel that is marked statically bound
						optimizer_handle_static_call(call, entity, env);
					}
					else {
						// dynamic call
						optimizer_handle_dynamic_call(call, entity, methodsel, env);
					}
				}
				else {
					// indirect call via function pointers or are there even more types of calls?
					DEBUGOUT(4, "\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
					DEBUGOUT(4, "-> %s", gdb_node_helper(callee));
					DEBUGOUT(4, "-> %s\n", gdb_node_helper(pred));
					if (get_Tuple_n_preds(pred) == 2 && is_Address(get_Tuple_pred(pred, 1))) {
						ir_entity *entity = get_Address_entity(get_Tuple_pred(pred, 1));

						if(is_method_entity(entity))
							optimizer_handle_static_call(call, entity, env);
					}
					else {
#ifdef XTA_STATS
						env->n_others++;
#endif
					}
				}
			}
			else {
				// indirect call via function pointers or are there even more types of calls?
				DEBUGOUT(4, "\tcall: neither Address nor Proj of MethodSel as callee: %s", gdb_node_helper(call));
				DEBUGOUT(4, "-> %s\n", gdb_node_helper(callee));
#ifdef XTA_STATS
				env->n_others++;
#endif
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
static void xta_devirtualize_calls(ir_entity **entry_points, cpmap_t *dyncall_targets)
{
	assert(dyncall_targets);

	pdeq *workqueue = new_pdeq();

	cpset_t done_set;
	cpset_init(&done_set, hash_ptr, ptr_equals);

	optimizer_env env = {
		.workqueue = workqueue,
		.done_set = &done_set,
		.dyncall_targets = dyncall_targets,
#ifdef XTA_STATS
		.n_staticcalls = 0,
		.n_dyncalls = 0,
		.n_icalls = 0,
		.n_devirts = 0,
		.n_devirts_icalls = 0,
		.n_others = 0,
		.n_reachmeth = 0,
#endif
	};

	{
		// add all given entry points to workqueue
		ir_entity *entity;
		for (size_t i = 0; (entity = entry_points[i]) != NULL; i++) {
			assert(is_method_entity(entity));
			ir_graph *graph = get_entity_irg(entity);
			assert(graph); // don't give methods without a graph as entry points for the analysis
			// add to workqueue
			pdeq_putr(workqueue, entity);
		}
	}

	while (!pdeq_empty(workqueue)) {
		ir_entity *entity = pdeq_getl(workqueue);
		assert(entity && is_method_entity(entity));

		if (cpset_find(&done_set, entity) != NULL) continue;

		DEBUGOUT(4, "\n== %s.%s (%s)\n", get_compound_name(get_entity_owner(entity)), get_entity_name(entity), get_entity_ld_name(entity));
#ifdef XTA_STATS
		env.n_reachmeth++;
#endif
		cpset_insert(&done_set, entity); // mark as done _before_ walking to not add it again in case of recursive calls
		ir_graph *graph = get_entity_irg(entity);
		if (graph == NULL) {
			optimizer_handle_no_graph(entity, &env);
		}
		else {
			irg_walk_graph(graph, NULL, walk_callgraph_and_devirtualize, &env);
		}
	}

#ifdef XTA_STATS
	printf("Reached methods\t\t %llu \n", env.n_reachmeth);
	printf("Static calls\t\t %llu \n", env.n_staticcalls);
	printf("Dyn calls\t\t %llu \n", env.n_dyncalls);
	printf("Interface calls\t\t %llu \n", env.n_icalls);
	printf("Other calls\t\t %llu \n", env.n_others);
	printf("Devirtualized calls\t\t %llu \n", env.n_devirts);
	printf("Devitualized interface calls\t\t %llu \n", env.n_devirts_icalls);
#endif

	// free data structures
	del_pdeq(workqueue);
	cpset_destroy(&done_set);
}


static void walk_graph_and_devirt_local(ir_node *node, void *env)
{
	int *count = env;

	switch (get_irn_opcode(node)) {
	case iro_Call: {
			ir_node *call = node;
			ir_node *callee = get_irn_n(call, 1);
			if (is_Proj(callee) && !oo_get_call_is_statically_bound(call)) {
				ir_node *pred = get_Proj_pred(callee);
				if (is_MethodSel(pred)) {
					ir_node *methodsel = pred;
					ir_entity *entity = get_MethodSel_entity(methodsel);
					ir_node *proj = get_irn_n(methodsel, 1);
					if (is_Proj(proj)) {
						ir_node *vptrIsSet = get_Proj_pred(proj);
						if (is_VptrIsSet(vptrIsSet)) {
							ir_type *type = get_VptrIsSet_type(vptrIsSet);
							ir_entity *target = get_class_member_by_name(type, get_entity_ident(entity));
							//printf("DEBUG: %s \n", get_entity_name(entity));
							if (target != NULL) {
								ir_graph *graph = get_irn_irg(methodsel);
								ir_node *address = new_r_Address(graph, target);
								ir_node *mem = get_irn_n(methodsel, 0);
								ir_node *input[] = { mem, address };
								turn_into_tuple(methodsel, 2, input);
								(*count)++;
							}
						}
					}
				}
			}
			break;
		}
	default: break;
	}

}

static void devirt_local(int *count)
{
	*count = 0;
	for (size_t i = 0, n = get_irp_n_irgs(); i < n; ++i) {
		ir_graph *irg = get_irp_irg(i);
		irg_walk_graph(irg, NULL, walk_graph_and_devirt_local, count);
	}
}

void xta_optimization(ir_entity **entry_points, ir_type **initial_live_classes, cpmap_t *ext_called_constr)
{
	assert(entry_points);
	cpmap_t dyncall_targets;


	int devirtualized_calls_local;
	devirt_local(&devirtualized_calls_local);


	cpmap_t empty_map;
	ir_timer_t *t = ir_timer_new();
	ir_timer_start(t);
	if(ext_called_constr == NULL) {
		ext_called_constr = &empty_map;
		cpmap_init(ext_called_constr, hash_ptr, ptr_equals);
	}
	xta_run(entry_points, initial_live_classes, &dyncall_targets, ext_called_constr);
	ir_timer_stop(t);
	xta_devirtualize_calls(entry_points, &dyncall_targets);
	xta_dispose_results(&dyncall_targets);

	DEBUGOUT(1, "Number initially devirtualized: %d\n", devirtualized_calls_local);
	DEBUGOUT(1, "XTA needed %f\n", ir_timer_elapsed_sec(t));
	ir_timer_free(t);
	DEBUGOUT(2, "Finished XTA\n");
	//Print program statistics
#ifdef XTA_STATS
	size_t n_classes = 0;
	size_t n_ext_classes = 0;
	size_t n_methods = 0;
	size_t n_ext_methods = 0;
	for (size_t i = 0, n = get_irp_n_types(); i < n; ++i) {
		ir_type *type = get_irp_type(i);
		if (is_Class_type(type)) {
			if (!strcmp(get_compound_name(type), "<frame_type>")) continue;
			if (!strcmp(get_compound_name(type), "array")) continue;
			if (oo_get_class_is_extern(type))
				n_ext_classes++;
			else
				n_classes++;
			for (size_t j = 0, k = get_class_n_members(type); j < k; ++j) {
				ir_entity *entity = get_class_member(type, j);
				if (oo_get_class_is_extern(type)) {
					if (is_method_entity(entity))
						n_ext_methods++;
				}
				else {
					if (is_method_entity(entity))
						n_methods++;
				}
			}
		}
	}
	printf("Count classes\t\t %lu \n", n_classes);
	printf("Count ext. classes\t\t  %lu \n", n_ext_classes);
	printf("Count methods\t\t %lu \n", n_methods);
	printf("Count ext. methods\t\t  %lu \n", n_ext_methods);
	printf("Count fields\t\t %lu \n", n_fields);
	printf("Count ext. fields\t\t %lu\n", n_ext_fields);
#endif
}
