#include <liboo/ddispatch.h>

#include <liboo/oo.h>
#include "adt/error.h"
#include <assert.h>

static ir_mode *mode_reference;
static ir_type *type_reference;

struct ddispatch_model_t {
	unsigned                      vptr_points_to_index;
	unsigned                      index_of_first_method;
	init_vtable_slots_t           init_vtable_slots;
	ident                        *abstract_method_ident;
	construct_interface_lookup_t  construct_interface_lookup;
} ddispatch_model;

static void __abstract_method(void)
{
	panic("Cannot invoke abstract method.");
}

static void default_init_vtable_slots(ir_type* klass, ir_initializer_t *vtable_init, unsigned vtable_size)
{
	(void) klass; (void) vtable_size;
	ir_graph *ccode_irg = get_const_code_irg();
	ir_node  *const_0   = new_r_Const_long(ccode_irg, mode_reference, 0);
	ir_initializer_t *slot_init = create_initializer_const(const_0);

	for (unsigned i = 0; i < ddispatch_model.index_of_first_method; i++) {
		set_initializer_compound_value(vtable_init, i, slot_init);
	}
}

static ir_node *default_interface_lookup_method(ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem)
{
	(void) objptr; (void) iface; (void) method; (void) irg; (void) block; (void) mem;
	panic("Invoking an interface method depends on runtime class information. Currently there's no default mechanism.");
	return NULL;
}

void ddispatch_init(void)
{
	mode_reference = mode_P;
	type_reference = new_type_primitive(mode_reference);

	ddispatch_model.vptr_points_to_index        = 0;
	ddispatch_model.index_of_first_method       = 0;
	ddispatch_model.init_vtable_slots           = default_init_vtable_slots;
	ddispatch_model.abstract_method_ident       = new_id_from_str("__abstract_method");
	ddispatch_model.construct_interface_lookup  = default_interface_lookup_method;

	(void) __abstract_method;
}

void ddispatch_setup_vtable(ir_type *klass)
{
	assert(is_Class_type(klass));

	if (! get_class_needs_vtable(klass))
		return;

	ident *vtable_name = mangle_vtable_name(klass);

	ir_type *global_type = get_glob_type();
	assert (get_class_member_by_name(global_type, vtable_name) == NULL);

	ir_type *superclass = NULL;
	unsigned vtable_size = ddispatch_model.index_of_first_method-ddispatch_model.vptr_points_to_index;
	int n_supertypes = get_class_n_supertypes(klass);
	if (n_supertypes > 0) {
		assert (n_supertypes == 1);
		superclass = get_class_supertype(klass, 0);
		vtable_size = get_class_vtable_size(superclass);
	}
	set_class_vtable_size(klass, vtable_size);

	// assign vtable ids
	for (int i = 0; i < get_class_n_members(klass); i++) {
		ir_entity *member = get_class_member(klass, i);
		if (is_method_entity(member)) {
			if (get_method_include_in_vtable(member)) {
				int n_overwrites = get_entity_n_overwrites(member);
				if (n_overwrites > 0) { // this method already has a vtable id, copy it from the superclass' implementation
					assert (n_overwrites == 1);
					ir_entity *overwritten_entity = get_entity_overwrites(member, 0);
					unsigned vtable_id = get_entity_vtable_number(overwritten_entity);
					assert (vtable_id != IR_VTABLE_NUM_NOT_SET);
					set_entity_vtable_number(member, vtable_id);
				} else {
					set_entity_vtable_number(member, vtable_size);
					set_class_vtable_size(klass, ++vtable_size);
				}
			}
		}
	}

	// the vtable currently is an array of pointers
	unsigned type_reference_size = get_type_size_bytes(type_reference);
	ir_type *vtable_type = new_type_array(1, type_reference);
	size_t vtable_ent_size = vtable_size + ddispatch_model.vptr_points_to_index;
	set_array_bounds_int(vtable_type, 0, 0, vtable_ent_size);
	set_type_size_bytes(vtable_type, type_reference_size * vtable_ent_size);
	set_type_state(vtable_type, layout_fixed);

	ir_entity *vtable = new_entity(global_type, vtable_name, vtable_type);
	set_entity_alignment(vtable, 32);

	ir_graph *const_code = get_const_code_irg();
	ir_initializer_t * init = create_initializer_compound(vtable_ent_size);

	if (superclass != NULL) {
		unsigned superclass_vtable_size = get_class_vtable_size(superclass);
		ir_entity *superclass_vtable_entity = get_class_member_by_name(global_type, mangle_vtable_name(superclass));
		assert (superclass_vtable_entity != NULL);
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
	for (int i = 0; i < get_class_n_members(klass); i++) {
		ir_entity *member = get_class_member(klass, i);
		if (is_method_entity(member)) {
			unsigned member_vtid = get_entity_vtable_number(member);
			if (member_vtid != IR_VTABLE_NUM_NOT_SET) {
				union symconst_symbol sym;
				if (! get_method_is_abstract(member)) {
					sym.entity_p = member;
				} else {
					sym.entity_p = new_entity(get_glob_type(), ddispatch_model.abstract_method_ident, get_entity_type(member));
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

	ir_node   *callee        = get_Call_ptr(call);
	if (! is_Sel(callee)) {
		if (is_SymConst_addr_ent(callee)
		 && get_SymConst_entity(callee) == dmemory_get_arraylength_entity())
			dmemory_lower_arraylength(call);
		return;
	}

	ir_node   *objptr        = get_Sel_ptr(callee);
	ir_entity *method_entity = get_Sel_entity(callee);

	if (! is_method_entity(method_entity))
		return;

	ir_type   *classtype     = get_entity_owner(method_entity);
	if (! is_Class_type(classtype))
		return;

	ddispatch_binding binding = get_method_binding(method_entity);
	if (binding == bind_unknown)
		return;

	ir_graph  *irg           = get_irn_irg(call);
	ir_node   *block         = get_nodes_block(call);
	ir_node   *cur_mem       = get_Call_mem(call);
	ir_node   *real_callee   = NULL;

	switch (binding) {
	case bind_static: {
		symconst_symbol callee_static;
		callee_static.entity_p = method_entity;
		real_callee = new_r_SymConst(irg, mode_reference, callee_static, symconst_addr_ent);
		break;
	}
	case bind_dynamic: {
		ir_node *vptr          = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, *get_class_vptr_entity_ptr(classtype));

		ir_node *vtable_load   = new_r_Load(block, cur_mem, vptr, mode_reference, cons_none);
		ir_node *vtable_addr   = new_r_Proj(vtable_load, mode_reference, pn_Load_res);
		cur_mem                = new_r_Proj(vtable_load, mode_M, pn_Load_M);

		unsigned vtable_id     = get_entity_vtable_number(method_entity);
		assert(vtable_id != IR_VTABLE_NUM_NOT_SET);

		unsigned type_ref_size = get_type_size_bytes(type_reference);
		ir_node *vtable_offset = new_r_Const_long(irg, mode_reference, vtable_id * type_ref_size);
		ir_node *funcptr_addr  = new_r_Add(block, vtable_addr, vtable_offset, mode_reference);
		ir_node *callee_load   = new_r_Load(block, cur_mem, funcptr_addr, mode_reference, cons_none);
		real_callee            = new_r_Proj(callee_load, mode_reference, pn_Load_res);
		cur_mem                = new_r_Proj(callee_load, mode_M, pn_Load_M);
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

void ddispatch_prepare_new_instance(ir_type* klass, ir_node *objptr, ir_graph *irg, ir_node *block, ir_node **mem)
{
	assert(is_Class_type(klass));

	ir_node   *cur_mem         = *mem;
	ir_node   *vptr            = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, *get_class_vptr_entity_ptr(klass));

	ir_type   *global_type     = get_glob_type();
	ir_entity *vtable_entity   = get_class_member_by_name(global_type, mangle_vtable_name(klass));

	union symconst_symbol sym;
	sym.entity_p = vtable_entity;
	ir_node   *vtable_symconst = new_r_SymConst(irg, mode_reference, sym, symconst_addr_ent);
	ir_node   *const_offset    = new_r_Const_long(irg, mode_reference, ddispatch_model.vptr_points_to_index * get_type_size_bytes(type_reference));
	ir_node   *vptr_target     = new_r_Add(block, vtable_symconst, const_offset, mode_reference);
	ir_node   *vptr_store      = new_r_Store(block, cur_mem, vptr, vptr_target, cons_none);
	cur_mem                    = new_r_Proj(vptr_store, mode_M, pn_Store_M);

	*mem = cur_mem;
}

void ddispatch_set_vtable_layout(unsigned vptr_points_to_index, unsigned index_of_first_method, init_vtable_slots_t func)
{
	assert (index_of_first_method >= vptr_points_to_index);
	assert (func);

	ddispatch_model.vptr_points_to_index  = vptr_points_to_index;
	ddispatch_model.index_of_first_method = index_of_first_method;
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
