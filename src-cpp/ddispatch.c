#include "config.h"

#include "liboo/ddispatch.h"
#include "liboo/oo.h"
#include "liboo/rtti.h"
#include "liboo/dmemory.h"

#include <assert.h>
#include "adt/error.h"

static ir_mode   *mode_reference;
static ir_type   *type_reference;

static ir_entity *default_lookup_interface_entity;

struct ddispatch_model_t {
	unsigned                      vptr_points_to_index;
	unsigned                      index_of_first_method;
	unsigned                      index_of_rtti_ptr;
	init_vtable_slots_t           init_vtable_slots;
	ident                        *abstract_method_ident;
	construct_interface_lookup_t  construct_interface_lookup;
} ddispatch_model;

static void default_init_vtable_slots(ir_type* klass, ir_initializer_t *vtable_init, unsigned vtable_size)
{
	(void) klass; (void) vtable_size;
	ir_graph         *ccode_irg = get_const_code_irg();

	ir_entity        *ci        = oo_get_class_rtti_entity(klass);
	assert (ci);

	symconst_symbol ci_sym;
	ci_sym.entity_p             = ci;
	ir_node          *ci_symc   = new_r_SymConst(ccode_irg, mode_P, ci_sym, symconst_addr_ent);
	ir_initializer_t *ci_init   = create_initializer_const(ci_symc);
	set_initializer_compound_value(vtable_init, ddispatch_model.index_of_rtti_ptr, ci_init);

	ir_node          *const_0   = new_r_Const_long(ccode_irg, mode_reference, 0);
	ir_initializer_t *slot_init = create_initializer_const(const_0);

	for (unsigned i = 0; i < ddispatch_model.index_of_first_method; i++) {
		if (i == ddispatch_model.index_of_rtti_ptr) continue;
		set_initializer_compound_value(vtable_init, i, slot_init);
	}
}

static ir_node *default_interface_lookup_method(ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node    *cur_mem        = *mem;

	// we need the reference to the object's class$ field
	// first, dereference the vptr in order to get the vtable address.
	ir_entity  *vptr_entity    = oo_get_class_vptr_entity(iface);
	ir_node    *vptr_addr      = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, vptr_entity);
	ir_node    *vptr_load      = new_r_Load(block, cur_mem, vptr_addr, mode_P, cons_none);
	ir_node    *vtable_addr    = new_r_Proj(vptr_load, mode_P, pn_Load_res);
	            cur_mem        = new_r_Proj(vptr_load, mode_M, pn_Load_M);

	// second, calculate the position of the RTTI ref in relation to the target of vptr and dereference it.
	int         offset         = (ddispatch_model.index_of_rtti_ptr - ddispatch_model.vptr_points_to_index) * get_type_size_bytes(type_reference);
	ir_node    *ci_offset      = new_r_Const_long(irg, mode_P, offset);
	ir_node    *ci_add         = new_r_Add(block, vtable_addr, ci_offset, mode_P);
	ir_node    *ci_load        = new_r_Load(block, cur_mem, ci_add, mode_P, cons_none);
	ir_node    *ci_ref         = new_r_Proj(ci_load, mode_P, pn_Load_res);
	            cur_mem        = new_r_Proj(ci_load, mode_M, pn_Load_M);

	const char *method_name    = get_entity_name(method);
	ir_entity  *name_const_ent = rtti_emit_string_const(method_name);
	symconst_symbol name_const_sym;
	name_const_sym.entity_p = name_const_ent;
	ir_node    *name_ref       = new_r_SymConst(irg, mode_P, name_const_sym, symconst_addr_ent);

	symconst_symbol callee_sym;
	callee_sym.entity_p      = default_lookup_interface_entity;
	ir_node   *callee        = new_r_SymConst(irg, mode_P, callee_sym, symconst_addr_ent);

	ir_node   *args[2]       = { ci_ref, name_ref };
	ir_type   *call_type     = get_entity_type(default_lookup_interface_entity);
	ir_node   *call          = new_r_Call(block, cur_mem, callee, 2, args, call_type);
	           cur_mem       = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node   *ress          = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node   *res           = new_r_Proj(ress, mode_P, 0);

	*mem = cur_mem;

	return res;
}

void ddispatch_init(void)
{
	mode_reference = mode_P;
	type_reference = new_type_primitive(mode_reference);

	ddispatch_model.vptr_points_to_index        = 0;
	ddispatch_model.index_of_first_method       = 1;
	ddispatch_model.index_of_rtti_ptr           = 0;
	ddispatch_model.init_vtable_slots           = default_init_vtable_slots;
	ddispatch_model.abstract_method_ident       = new_id_from_str("oo_rt_abstract_method_error");
	ddispatch_model.construct_interface_lookup  = default_interface_lookup_method;

	ir_type *default_li_type = new_type_method(2, 1);
	set_method_param_type(default_li_type, 0, type_reference);
	set_method_param_type(default_li_type, 1, type_reference);
	set_method_res_type(default_li_type, 0, type_reference);
	ident *default_li_ident = new_id_from_str("oo_rt_lookup_interface_method");
	default_lookup_interface_entity
		= create_compilerlib_entity(default_li_ident, default_li_type);
}

void ddispatch_setup_vtable(ir_type *klass)
{
	assert(is_Class_type(klass));

	ir_entity *vtable = oo_get_class_vtable_entity(klass);
	if (! vtable)
		return;

	unsigned vtable_size;
	ir_type *superclass = oo_get_class_superclass(klass);
	if (superclass) {
		vtable_size = oo_get_class_vtable_size(superclass);
	} else {
		vtable_size = ddispatch_model.index_of_first_method-ddispatch_model.vptr_points_to_index;
	}

	// assign vtable ids
	for (size_t i = 0; i < get_class_n_members(klass); i++) {
		ir_entity *member = get_class_member(klass, i);
		if (! is_method_entity(member))
			continue;
		if (oo_get_method_exclude_from_vtable(member))
			continue;

		ir_entity *overwritten_entity = oo_get_entity_overwritten_superclass_entity(member);
		if (overwritten_entity) {
			int vtable_id = oo_get_method_vtable_index(overwritten_entity);
			assert (vtable_id != -1);
			oo_set_method_vtable_index(member, vtable_id);
		} else {
			// assign new vtable id
			oo_set_method_vtable_index(member, vtable_size);
			++vtable_size;
		}
	}
	oo_set_class_vtable_size(klass, vtable_size);

	// the vtable currently is an array of pointers
	unsigned type_reference_size = get_type_size_bytes(type_reference);
	ir_type *vtable_type = new_type_array(1, type_reference);
	size_t vtable_ent_size = vtable_size + ddispatch_model.vptr_points_to_index;
	set_array_bounds_int(vtable_type, 0, 0, vtable_ent_size);
	set_type_size_bytes(vtable_type, type_reference_size * vtable_ent_size);
	set_type_state(vtable_type, layout_fixed);
	set_entity_type(vtable, vtable_type);
	set_entity_alignment(vtable, 32);

	ir_graph *const_code = get_const_code_irg();
	ir_initializer_t * init = create_initializer_compound(vtable_ent_size);

	if (superclass != NULL) {
		unsigned   superclass_vtable_size   = oo_get_class_vtable_size(superclass);
		ir_entity *superclass_vtable_entity = oo_get_class_vtable_entity(superclass);
		assert (superclass_vtable_entity);
		ir_initializer_t *superclass_vtable_init = get_entity_initializer(superclass_vtable_entity);

		// copy vtable initialization from superclass
		for (unsigned i = ddispatch_model.vptr_points_to_index;
			          i < superclass_vtable_size+ddispatch_model.vptr_points_to_index;
			          i++) {
				ir_initializer_t *superclass_vtable_init_value = get_initializer_compound_value(superclass_vtable_init, i);
				set_initializer_compound_value (init, i, superclass_vtable_init_value);
		}
	}

	// setup / replace vtable entries to point to clazz's implementation
	for (size_t i = 0; i < get_class_n_members(klass); i++) {
		ir_entity *member = get_class_member(klass, i);
		if (is_method_entity(member)) {
			int member_vtid = oo_get_method_vtable_index(member);
			if (member_vtid != -1) {
				union symconst_symbol sym;
				if (oo_get_method_is_inherited(member)) {
					ir_entity *impl = oo_get_entity_overwritten_superclass_entity(member);
					assert (impl);
					sym.entity_p = impl;
				} else if (! oo_get_method_is_abstract(member)) {
					sym.entity_p = member;
				} else {
					ident     *id     = ddispatch_model.abstract_method_ident;
					ir_entity *entity = create_compilerlib_entity(id, get_entity_type(member));
					sym.entity_p = entity;
				}
				ir_node *symconst_node = new_r_SymConst(const_code, mode_reference, sym, symconst_addr_ent);
				ir_initializer_t *val = create_initializer_const(symconst_node);
				set_initializer_compound_value (init, member_vtid+ddispatch_model.vptr_points_to_index, val);
			}
		}
	}

	(*ddispatch_model.init_vtable_slots)(klass, init, vtable_ent_size);

	set_entity_initializer(vtable, init);
}

void ddispatch_lower_Call(ir_node* call)
{
	assert(is_Call(call));

	ir_node *callee = get_Call_ptr(call);
	if (!is_Sel(callee))
		return;

	ir_node   *objptr        = get_Sel_ptr(callee);
	ir_entity *method_entity = get_Sel_entity(callee);
	if (!is_method_entity(method_entity))
		return;

	ir_type *classtype = get_entity_owner(method_entity);
	if (!is_Class_type(classtype))
		return;

	ddispatch_binding binding = oo_get_entity_binding(method_entity);
	if (binding == bind_unknown)
		panic("method %s has no binding specified",
		      get_entity_name(method_entity));

	/* If the call has been explicitly marked as statically bound, then obey. */
	if (oo_get_call_is_statically_bound(call))
		binding = bind_static;

	ir_graph *irg         = get_irn_irg(call);
	ir_node  *block       = get_nodes_block(call);
	ir_node  *cur_mem     = get_Call_mem(call);
	ir_node  *real_callee = NULL;

	switch (binding) {
	case bind_static: {
		symconst_symbol callee_static;
		callee_static.entity_p = method_entity;
		real_callee = new_r_SymConst(irg, mode_reference, callee_static, symconst_addr_ent);
		break;
	}
	case bind_dynamic: {
		ir_entity *vptr_entity  = oo_get_class_vptr_entity(classtype);
		ir_node   *vptr         = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, vptr_entity);

		ir_node   *vtable_load  = new_r_Load(block, cur_mem, vptr, mode_reference, cons_none);
		ir_node   *vtable_addr  = new_r_Proj(vtable_load, mode_reference, pn_Load_res);
		cur_mem                 = new_r_Proj(vtable_load, mode_M, pn_Load_M);

		int        vtable_id    = oo_get_method_vtable_index(method_entity);
		assert(vtable_id != -1);

		unsigned type_ref_size  = get_type_size_bytes(type_reference);
		ir_node *vtable_offset  = new_r_Const_long(irg, mode_reference, vtable_id * type_ref_size);
		ir_node *funcptr_addr   = new_r_Add(block, vtable_addr, vtable_offset, mode_reference);
		ir_node *callee_load    = new_r_Load(block, cur_mem, funcptr_addr, mode_reference, cons_none);
		real_callee             = new_r_Proj(callee_load, mode_reference, pn_Load_res);
		cur_mem                 = new_r_Proj(callee_load, mode_M, pn_Load_M);
		break;
	}
	case bind_interface: {
		real_callee = (*ddispatch_model.construct_interface_lookup)(objptr, classtype, method_entity, irg, block, &cur_mem);
		break;
	}
	default:
		panic("Cannot lower call.");
	}

	set_Call_ptr(call, real_callee);
	set_Call_mem(call, cur_mem);
}

void ddispatch_prepare_new_instance(dbg_info *dbgi, ir_node *block, ir_node *objptr, ir_node **mem, ir_type* klass)
{
	ir_graph *irg = get_irn_irg(block);
	assert(is_Class_type(klass));

	ir_node   *cur_mem         = *mem;
	ir_entity *vptr_entity     = oo_get_class_vptr_entity(klass);
	ir_node   *vptr            = new_rd_Sel(dbgi, block, new_r_NoMem(irg), objptr, 0, NULL, vptr_entity);

	ir_node   *vptr_target     = NULL;
	ir_entity *vtable_entity   = oo_get_class_vtable_entity(klass);
	if (vtable_entity) {
		union symconst_symbol sym;
		sym.entity_p = vtable_entity;
		ir_node   *vtable_symconst = new_r_SymConst(irg, mode_reference, sym, symconst_addr_ent);
		ir_node   *const_offset    = new_r_Const_long(irg, mode_reference, ddispatch_model.vptr_points_to_index * get_type_size_bytes(type_reference));
		vptr_target                = new_rd_Add(dbgi, block, vtable_symconst, const_offset, mode_reference);
	} else {
		vptr_target                = new_r_Const_long(irg, mode_P, 0);
	}

	ir_node   *vptr_store      = new_rd_Store(dbgi, block, cur_mem, vptr, vptr_target, cons_floats);
	cur_mem                    = new_r_Proj(vptr_store, mode_M, pn_Store_M);

	*mem = cur_mem;
}

void ddispatch_set_vtable_layout(unsigned vptr_points_to_index, unsigned index_of_first_method, unsigned index_of_rtti_ptr, init_vtable_slots_t func)
{
	assert (index_of_first_method >= vptr_points_to_index);
	assert (func);

	ddispatch_model.vptr_points_to_index  = vptr_points_to_index;
	ddispatch_model.index_of_first_method = index_of_first_method;
	ddispatch_model.index_of_rtti_ptr     = index_of_rtti_ptr;
	ddispatch_model.init_vtable_slots     = func;
}

void ddispatch_set_interface_lookup_constructor(construct_interface_lookup_t func)
{
	assert (func);
	ddispatch_model.construct_interface_lookup = func;
}

void ddispatch_set_abstract_method_ident(ident* ami)
{
	assert (ami);
	ddispatch_model.abstract_method_ident = ami;
}

unsigned ddispatch_get_vptr_points_to_index()
{
	return ddispatch_model.vptr_points_to_index;
}

unsigned ddispatch_get_index_of_rtti_ptr()
{
	return ddispatch_model.index_of_rtti_ptr;
}
