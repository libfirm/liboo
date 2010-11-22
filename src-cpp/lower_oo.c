#include <liboo/lower_oo.h>

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <libfirm/firm.h>

#include "adt/error.h"

#include <liboo/mangle.h>

static ir_entity *calloc_entity;
static ir_entity *vptr_entity;
// there's exactly one vptr entity, member of java.lang.Object.

static ident *abstract_method_ident;

static ir_mode *mode_reference;
static ir_type *type_reference;

static void move_to_global(ir_entity *entity)
{
	/* move to global type */
	ir_type *owner = get_entity_owner(entity);
	assert(is_Class_type(owner));
	ir_type *global_type = get_glob_type();
	set_entity_owner(entity, global_type);
}

static void lower_type(type_or_ent tore, void *env)
{
	(void) env;
	if (get_kind(tore.typ) != k_type)
		return;

	ir_type *type = tore.typ;
	if (!is_Class_type(type)) {
		set_type_state(type, layout_fixed);
		return;
	}

	/* mangle entity names */
	int n_members = get_class_n_members(type);
	for (int m = 0; m < n_members; ++m) {
		ir_entity *entity = get_class_member(type, m);
		/* don't mangle names of entities with explicitely set names */
		if (entity_has_ld_ident(entity)) {
			continue;
		}
		ident *mangled_id = mangle_entity_name(entity);
		set_entity_ld_ident(entity, mangled_id);
	}

	ir_type *global_type = get_glob_type();
	if (type == global_type)
		return;

	n_members = get_class_n_members(type);
	for (int m = n_members-1; m >= 0; --m) {
		ir_entity *entity = get_class_member(type, m);
		if (is_method_entity(entity) ||
				get_entity_allocation(entity) == allocation_static) {
			move_to_global(entity);
		}
	}

	/* layout fields */
	default_layout_compound_type(type);
}

#if 0
static void lower_Alloc(ir_node *node)
{
	assert(is_Alloc(node));

	if (get_Alloc_where(node) != heap_alloc)
		return;

	ir_graph *irg   = get_irn_irg(node);
	ir_type  *type  = get_Alloc_type(node);
	ir_node  *count = get_Alloc_count(node);

	ir_node  *res   = NULL;

	ir_node  *cur_mem = get_Alloc_mem(node);
	ir_node  *block   = get_nodes_block(node);

	ir_type  *eltype   = is_Array_type(type) ? get_array_element_type(type) : NULL;

	if (is_Class_type(type)) {
		res = gcji_allocate_object(type, irg, block, &cur_mem);
	} else if (is_Array_type(type)) {
		res = gcji_allocate_array(eltype, count, irg, block, &cur_mem);
	} else {
		assert (0);
	}

	if (is_Class_type(type)) {
		ir_node   *vptr          = new_r_Sel(block, new_r_NoMem(irg), res, 0, NULL, vptr_entity);

		ir_type   *global_type   = get_glob_type();
		ir_entity *vtable_entity = get_class_member_by_name(global_type, mangle_vtable_name(type));

		union symconst_symbol sym;
		sym.entity_p = vtable_entity;
		ir_node   *vtable_symconst = new_r_SymConst(irg, mode_reference, sym, symconst_addr_ent);
		ir_node   *const_offset    = new_r_Const_long(irg, mode_reference, GCJI_VTABLE_OFFSET * get_type_size_bytes(type_reference));
		ir_node   *vptr_target     = new_r_Add(block, vtable_symconst, const_offset, mode_reference);
		ir_node   *vptr_store      = new_r_Store(block, cur_mem, vptr, vptr_target, cons_none);
		           cur_mem         = new_r_Proj(vptr_store, mode_M, pn_Store_M);
	}

	turn_into_tuple(node, pn_Alloc_max);
	set_irn_n(node, pn_Alloc_M, cur_mem);
	set_irn_n(node, pn_Alloc_res, res);
}

static void lower_arraylength(ir_node *call)
{
	ir_node  *array_ref = get_Call_param(call, 0);

	ir_node  *block     = get_nodes_block(call);
	ir_graph *irg       = get_irn_irg(block);

	ir_node  *cur_mem   = get_Call_mem(call);

	ir_node  *len       = gcji_get_arraylength(array_ref, irg, block, &cur_mem);

	ir_node  *in[]      = { len };
	ir_node  *lent      = new_r_Tuple(block, sizeof(in)/sizeof(*in), in);

	turn_into_tuple(call, pn_Call_max);
	set_irn_n(call, pn_Call_M, cur_mem);
	set_irn_n(call, pn_Call_T_result, lent);
}
#endif

static void lower_Call(ir_node* call)
{
	assert(is_Call(call));

	ir_node *callee = get_Call_ptr(call);
#if 0
	if (is_SymConst(callee)
			&& get_SymConst_entity(callee) == builtin_arraylength) {
		lower_arraylength(call);
		return;
	}
#endif

	if (! is_Sel(callee))
		return;

	ir_node *objptr = get_Sel_ptr(callee);
	ir_entity *method_entity = get_Sel_entity(callee);
	if (! is_method_entity(method_entity))
		return;

	ir_type *classtype    = get_entity_owner(method_entity);
	if (! is_Class_type(classtype))
		return;

	//uint16_t cl_access_flags = ((class_t*)get_type_link(classtype))->access_flags;
	//uint16_t mt_access_flags = ((method_t*)get_entity_link(method_entity))->access_flags;

	ir_graph *irg         = get_irn_irg(call);
	ir_node  *block       = get_nodes_block(call);
	ir_node  *cur_mem     = get_Call_mem(call);
	ir_node  *real_callee = NULL;

#if 0
	if ((cl_access_flags & ACCESS_FLAG_INTERFACE) != 0) {
		real_callee           = gcji_lookup_interface(objptr, classtype, method_entity, irg, block, &cur_mem);
	} else {
#endif
		//int link_static       = (cl_access_flags & ACCESS_FLAG_FINAL) + (mt_access_flags & ACCESS_FLAG_FINAL) != 0;
		int link_static = 0;

		if (! link_static) {
			ir_node *vptr         = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, vptr_entity);


			ir_node *vtable_load  = new_r_Load(block, cur_mem, vptr, mode_P, cons_none);
			ir_node *vtable_addr  = new_r_Proj(vtable_load, mode_P, pn_Load_res);
					 cur_mem      = new_r_Proj(vtable_load, mode_M, pn_Load_M);

			unsigned vtable_id    = get_entity_vtable_number(method_entity);
			assert(vtable_id != IR_VTABLE_NUM_NOT_SET);

			unsigned type_reference_size = get_type_size_bytes(type_reference);
			ir_node *vtable_offset= new_r_Const_long(irg, mode_P, vtable_id * type_reference_size);
			ir_node *funcptr_addr = new_r_Add(block, vtable_addr, vtable_offset, mode_P);
			ir_node *callee_load  = new_r_Load(block, cur_mem, funcptr_addr, mode_P, cons_none);
					 real_callee  = new_r_Proj(callee_load, mode_P, pn_Load_res);
					 cur_mem      = new_r_Proj(callee_load, mode_M, pn_Load_M);
		} else {
			symconst_symbol callee;
			callee.entity_p = method_entity;
			real_callee = new_r_SymConst(irg, mode_reference, callee, symconst_addr_ent);
		}
	//}

	set_Call_ptr(call, real_callee);
	set_Call_mem(call, cur_mem);
}

static void lower_node(ir_node *node, void *env)
{
	(void) env;
	if (is_Alloc(node)) {
		//lower_Alloc(node);
	} else if (is_Call(node)) {
		lower_Call(node);
	}
}

static void lower_graph(ir_graph *irg)
{
	irg_walk_graph(irg, NULL, lower_node, NULL);
}

/**
 * Lower object oriented constructs
 */
void lower_oo(void)
{
	abstract_method_ident = new_id_from_str("_Jv_ThrowAbstractMethodError");

	mode_reference = mode_P;
	type_reference = new_type_primitive(mode_reference);

	//class_walk_super2sub(setup_vtable, NULL, NULL);

	ir_type *method_type = new_type_method(2, 1);
	ir_type *t_size_t    = new_type_primitive(mode_Iu);
	ir_type *t_ptr       = new_type_primitive(mode_reference);
	set_method_param_type(method_type, 0, t_size_t);
	set_method_param_type(method_type, 1, t_size_t);
	set_method_res_type(method_type, 0, t_ptr);
	add_method_additional_properties(method_type, mtp_property_malloc);

	ir_type *glob = get_glob_type();
	ident   *id   = new_id_from_str("calloc");
	calloc_entity = new_entity(glob, id, method_type);
	set_entity_ld_ident(calloc_entity, id);
	set_entity_visibility(calloc_entity, ir_visibility_external);
	add_method_additional_properties(method_type, mtp_property_malloc);

	int n_irgs = get_irp_n_irgs();
	for (int i = 0; i < n_irgs; ++i) {
		ir_graph *irg = get_irp_irg(i);
		lower_graph(irg);
	}

	type_walk_super2sub(lower_type, NULL, NULL);

	//dump_all_ir_graphs("before_highlevel");
	lower_highlevel(0);
}
