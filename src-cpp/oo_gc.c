
/*
 * This file is part of liboo. And is a liboo-compatible garbage collector.
 * Despite based on firm's gc this one is working on the Whole Program Asssumption and walks from the given methods onwards.
 * Copyright (C) 2012 University of Karlsruhe.
 */

/**
 * @file
 * @brief    Removal of unreachable methods before oo-lowering.
 * @author   Daniel Biester
 */
#include "liboo/oo_gc.h"

#include <assert.h>
#include <liboo/oo.h>

#include "adt/cpset.h"
#include "adt/hashptr.h"
//DEBUG_ONLY(static firm_dbg_module_t *dbg;)

#define foreach_irp_irg_r(idx, irg) \
	for (bool irg##__b = true; irg##__b;) \
		for (; irg##__b; irg##__b = false) \
			for (size_t idx = get_irp_n_irgs(); irg##__b && idx-- != 0;) \
				for (ir_graph *const irg = (irg##__b = false, get_irp_irg(idx)); !irg##__b; irg##__b = true)

static int ptr_equals(const void *pt1, const void *pt2) // missing default pointer compare function
{
	return pt1 == pt2;
}

static ir_entity *get_class_member_by_name(ir_type *cls, ident *ident) // function which was removed from newer libfirm versions
{
	for (size_t i = 0, n = get_class_n_members(cls); i < n; ++i) {
		ir_entity *entity = get_class_member(cls, i);
		if (get_entity_ident(entity) == ident)
			return entity;
	}
	return NULL;
}

static void visit_entity(ir_entity *entity);

static void visit_node(ir_node *node, void *env)
{
	(void)env;

	ir_entity *entity = get_irn_entity_attr(node);
	visit_entity(entity);
}

static void start_visit_node(ir_node *node)
{
	ir_graph *irg = get_irn_irg(node);
	if (get_irg_visited(irg) < get_max_irg_visited()) {
		set_irg_visited(irg, get_max_irg_visited());
	}
	irg_walk_2(node, visit_node, NULL, NULL);
}

static void visit_initializer(ir_initializer_t *initializer)
{
	switch (get_initializer_kind(initializer)) {
	case IR_INITIALIZER_CONST:
		start_visit_node(get_initializer_const_value(initializer));
		return;
	case IR_INITIALIZER_TARVAL:
	case IR_INITIALIZER_NULL:
		return;

	case IR_INITIALIZER_COMPOUND: {
		for (size_t i = 0; i < get_initializer_compound_n_entries(initializer); ++i) {
			ir_initializer_t *subinitializer
				= get_initializer_compound_value(initializer, i);
			visit_initializer(subinitializer);
		}
		return;
	}
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


	result = get_class_member_by_name(klass, get_entity_ident(call_entity));

	if (result != NULL) {
		if (oo_get_method_is_abstract(result)) {
			result = NULL;
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
					//Nothing to do?
				}
				else if (r_overrides_result || (result_from_interface && !r_from_interface)) {
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

static void collect_methods_recursive(ir_entity *call_entity, ir_type *klass, ir_entity *entity, cpset_t *result_set)
{
	assert(call_entity);
	assert(is_method_entity(call_entity));
	assert(klass);
	assert(is_Class_type(klass));
	assert(entity);
	assert(is_method_entity(entity));
	assert(result_set);

	ir_entity *current_entity = entity;

	ir_entity *overwriting_entity = get_class_member_by_name(klass, get_entity_ident(current_entity));
	if (overwriting_entity != NULL && overwriting_entity != current_entity) { // if has overwriting entity

		current_entity = overwriting_entity;
	}
	// else it is inherited


	// support for FIRM usage without any entity copies at all (not even for case interface method implemention inherited from a superclass) -> have to assume some usual semantics
	// for interface calls (or more general abstract calls) there has to be a non-abstract implementation in each non-abstract subclass, if there is no entity copy we have to find the implementation by ourselves (in cases an inherited method implements the abstract method)
	if (oo_get_method_is_abstract(call_entity) && !oo_get_class_is_abstract(klass) && !oo_get_class_is_interface(klass) && oo_get_method_is_abstract(current_entity)) { // careful: interfaces seem not always to be marked as abstract
		ir_entity *inherited_impl = find_inherited_implementation(klass, call_entity);
		if (inherited_impl != NULL) {
			current_entity = inherited_impl;
		}
	}
	if (!oo_get_method_is_abstract(current_entity)) { // ignore abstract methods
		cpset_insert(result_set, current_entity);
	}

	size_t n_subtypes = get_class_n_subtypes(klass);
	for (size_t i = 0; i < n_subtypes; i++) {
		ir_type *subclass = get_class_subtype(klass, i);

		collect_methods_recursive(call_entity, subclass, current_entity, result_set);
	}
}

// collect method entities from downwards in the class hierarchy
// it walks down the classes to have the entities with the classes even when the method is inherited
static void collect_methods(ir_entity *call_entity, cpset_t *result_set)
{
	collect_methods_recursive(call_entity, get_entity_owner(call_entity), call_entity, result_set);
}
static void visit_overwrittenby_entities(ir_entity *entity) {
	if(get_entity_owner(entity) == get_glob_type())
		return;
	cpset_t result_set;
	cpset_init(&result_set, hash_ptr, ptr_equals);
	collect_methods(entity, &result_set);
	cpset_iterator_t it;
	cpset_iterator_init(&it, &result_set);
	ir_entity *overwrittenby;
	while((overwrittenby = cpset_iterator_next(&it)) != NULL) {
		visit_entity(overwrittenby);
	}
	cpset_destroy(&result_set);
}

static void visit_entity(ir_entity *entity)
{
	if (entity == NULL || entity_visited(entity))
		return;

	mark_entity_visited(entity);

	if(is_method_entity(entity)) {
		visit_overwrittenby_entities(entity);
		ir_graph *irg = get_entity_irg(entity);
		if (irg != NULL)
			start_visit_node(get_irg_end(irg));
	} else if(is_alias_entity(entity)) {
		ir_entity *aliased = get_entity_alias(entity);
		visit_entity(aliased);
	} else if(entity_has_definition(entity)) {
		ir_initializer_t *const init = get_entity_initializer(entity);
		if (init)
			visit_initializer(init);
	}
}

static void garbage_collect_in_segment(ir_type *segment)
{
	for (int i = get_compound_n_members(segment); i-- > 0; ) {
		ir_entity *entity = get_compound_member(segment, i);
		if (entity_visited(entity))
			continue;

		free_entity(entity);
	}
}

static void adjust_overwrites_at_deletion(ir_entity *entity) {
	size_t n_overwrites = get_entity_n_overwrites(entity);
	
	for(size_t i = 0; i< n_overwrites; ++i) {
		ir_entity *overwrites = get_entity_overwrites(entity, i);
		remove_entity_overwrittenby(overwrites, entity);
	}
}

static void adjust_overwrittenby_at_deletion(ir_entity *entity) {
	size_t n_overwritten = get_entity_n_overwrittenby(entity);

	for(size_t i = 0; i< n_overwritten; ++i) {
		ir_entity *overwrittenby = get_entity_overwrittenby(entity, i);
		remove_entity_overwrites(overwrittenby, entity);
		
		for(size_t j = 0; j < get_entity_n_overwrites(entity); ++j) {
			ir_entity *overwrites = get_entity_overwrites(entity, j);
			add_entity_overwrites(overwrittenby, overwrites);
		}
		
		if(get_entity_n_overwrites(overwrittenby) == 0) {
			oo_set_method_is_inherited(overwrittenby, false);
		}
	}
}

static void garbage_collect_in_class(ir_type *klass) {
	int n_members = get_class_n_members(klass);
	for (int m = n_members-1; m >= 0; m--) {
		ir_entity *ent = get_class_member(klass, m);
		if(!is_method_entity(ent))
			continue;
		if(entity_visited(ent))
			continue;
		adjust_overwrites_at_deletion(ent);
		adjust_overwrittenby_at_deletion(ent);
		if(get_entity_irg(ent)) 
			free_ir_graph(get_entity_irg(ent));
		free_entity(ent);
	}

}

static void visit_oo_type_info_entities(ir_type *klass) {
	visit_entity(oo_get_class_vtable_entity(klass));
	visit_entity(oo_get_class_vptr_entity(klass));
	visit_entity(oo_get_class_rtti_entity(klass));
	visit_entity(oo_get_class_itt_entity(klass));
}

void oo_garbage_collect_entities(ir_entity **entry_points)
{
	/* start a type walk for all externally visible entities */
	irp_reserve_resources(irp, IRP_RESOURCE_TYPE_VISITED);
	inc_master_type_visited();
	inc_max_irg_visited();

	
	ir_entity *entity;
	for (size_t i = 0; (entity = entry_points[i]) != NULL; i++) {
		visit_entity(entity);
	}

	/* 
	 * Delete non-visited entites in oo structs
	 */
	for (size_t i = 0, n = get_irp_n_types(); i < n; ++i) {
		ir_type *type = get_irp_type(i);
		if (is_Class_type(type)) {
			visit_oo_type_info_entities(type);
			garbage_collect_in_class(type);
		}
	}
	/* remove graphs of non-visited functions
	 * (we have to count backwards, because freeing the graph moves the last
	 *  graph in the list to the free position) */
	foreach_irp_irg_r(i, irg) {
		ir_entity *entity = get_irg_entity(irg);

		if (entity_visited(entity))
			continue;

		free_ir_graph(irg);
	}

	/* we can now remove all non-visited (global) entities */
	for (ir_segment_t s = IR_SEGMENT_FIRST; s <= IR_SEGMENT_LAST; ++s) {
		ir_type *type = get_segment_type(s);
		garbage_collect_in_segment(type);
	}
	irp_free_resources(irp, IRP_RESOURCE_TYPE_VISITED);
}
