
#include <liboo/ddispatch.h>
#include <liboo/mangle.h>

#include <assert.h>

static ir_mode *mode_reference;
static ir_type *type_reference;

static ddispatch_params ddp;

void ddispatch_init(ddispatch_params params)
{
	ddp = params;

	mode_reference = mode_reference;
	type_reference = new_type_primitive(mode_reference);
}

void ddispatch_setup_vtable(ir_type *klass)
{
	assert(is_Class_type(klass));

	if (! (*ddp.vtable_create_pred)(klass))
		return;

	ident *vtable_name = mangle_vtable_name(klass);

	ir_type *global_type = get_glob_type();
	assert (get_class_member_by_name(global_type, vtable_name) == NULL);

	ir_type *superclass = NULL;
	unsigned vtable_size = ddp.vtable_index_of_first_method;
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
			if ((*ddp.vtable_include_pred)(member)) {
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
	size_t vtable_ent_size = vtable_size + ddp.vtable_vptr_points_to_index;
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
		for (unsigned i = ddp.vtable_vptr_points_to_index; i < superclass_vtable_size+ddp.vtable_vptr_points_to_index; i++) {
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
				if (! (*ddp.vtable_is_abstract_pred)(member)) {
					sym.entity_p = member;
				} else {
					sym.entity_p = new_entity(get_glob_type(), ddp.vtable_abstract_method_ident, get_entity_type(member));
				}
				ir_node *symconst_node = new_r_SymConst(const_code, mode_reference, sym, symconst_addr_ent);
				ir_initializer_t *val = create_initializer_const(symconst_node);
				set_initializer_compound_value (init, member_vtid+ddp.vtable_vptr_points_to_index, val);
			}
		}
	}

	(*ddp.vtable_init_slots)(klass, init, vtable_ent_size);

	set_entity_initializer(vtable, init);
}

void ddispatch_lower_Call(ir_node* call)
{
	assert(is_Call(call));

	ddispatch_binding binding = (*ddp.call_decide_binding)(call);

	if (binding == bind_already_bound)
		return;

	if (binding == bind_builtin) {
		(*ddp.call_lower_builtin)(call);
		return;
	}

	ir_node   *callee        = get_Call_ptr(call);
	assert(is_Sel(callee));

	ir_node   *objptr        = get_Sel_ptr(callee);
	ir_entity *method_entity = get_Sel_entity(callee);
	if (! is_method_entity(method_entity))
		return;

	ir_type   *classtype     = get_entity_owner(method_entity);
	if (! is_Class_type(classtype))
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
		ir_node *vptr          = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, ddp.call_vptr_entity);

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
		real_callee = (*ddp.call_lookup_interface_method)(objptr, classtype, method_entity, irg, block, &cur_mem);
		break;
	}
	default:
		assert(0);
	}

	set_Call_ptr(call, real_callee);
	set_Call_mem(call, cur_mem);
}
