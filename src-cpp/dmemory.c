#include "config.h"

#include "liboo/dmemory.h"

#include <assert.h>
#include "liboo/ddispatch.h"
#include "adt/error.h"

static ir_op *op_Arraylength;

enum {
	dmemory_pos_Arraylength_mem = 0,
	dmemory_pos_Arraylength_arrayref = 1
};

static void dump_node(FILE *f, ir_node *irn, dump_reason_t reason)
{
	assert (is_Arraylength(irn));
	switch (reason) {
	case dump_node_opcode_txt:
		fputs(get_op_name(get_irn_op(irn)), f);
		break;
	case dump_node_mode_txt:
		break;
	case dump_node_nodeattr_txt:
		break;
	case dump_node_info_txt:
		break;
	default:
		break;
	}
}

static const ir_op_ops dmemory_node_op_ops = {
	firm_default_hash,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	dump_node,
	NULL,
	NULL
};

ir_node *new_Arraylength(ir_node* mem, ir_node *arrayref)
{
	ir_graph *irg   = get_irn_irg(arrayref);
	ir_node  *block = get_nodes_block(arrayref);

	ir_node  *in[2];
	in[dmemory_pos_Arraylength_mem]      = mem;
	in[dmemory_pos_Arraylength_arrayref] = arrayref;

	ir_node *res = new_ir_node(NULL, irg, block, op_Arraylength, mode_T, 2,	in);

	return res;
}

ir_node *get_Arraylength_mem(const ir_node *node)
{
	assert (is_Arraylength(node));
	return get_irn_n(node, dmemory_pos_Arraylength_mem);
}

ir_node *get_Arraylength_arrayref(const ir_node *node)
{
	assert (is_Arraylength(node));
	return get_irn_n(node, dmemory_pos_Arraylength_arrayref);
}

bool is_Arraylength(const ir_node *node)
{
	return get_irn_op(node) == op_Arraylength;
}

struct dmemory_model_t {
	alloc_object_t    alloc_object;
	alloc_array_t     alloc_array;
	get_arraylength_t get_arraylength;
} dmemory_model;

static ir_entity *calloc_entity;
static ir_mode   *default_arraylength_mode;

static ir_node *default_alloc_object(ir_type *type, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node  *cur_mem = *mem;
	symconst_symbol type_sym;
	type_sym.type_p = type;
	ir_node  *size = new_r_SymConst(irg, mode_Iu, type_sym, symconst_type_size);

	symconst_symbol calloc_sym;
	calloc_sym.entity_p = calloc_entity;
	ir_node *callee = new_r_SymConst(irg, mode_P, calloc_sym, symconst_addr_ent);

	ir_node *one       = new_r_Const_long(irg, mode_Iu, 1);
	ir_node *in[2]     = { one, size };
	ir_type *call_type = get_entity_type(calloc_entity);
	ir_node *call      = new_r_Call(block, cur_mem, callee, 2, in, call_type);
	cur_mem            = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node *ress      = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node *res       = new_r_Proj(ress, mode_P, 0);

	*mem = cur_mem;
	return res;
}

static ir_node *default_alloc_array(ir_type *eltype, ir_node *count, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node *cur_mem      = *mem;

	unsigned count_size   = get_mode_size_bytes(default_arraylength_mode);
	unsigned element_size = is_Class_type(eltype) ? get_mode_size_bytes(mode_P) : get_type_size_bytes(eltype); // FIXME: some langs support arrays of structs.
	/* increase element count so we have enough space for a counter
			   at the front */
	unsigned add_size     = (element_size + (count_size-1)) / count_size;
	ir_node *count_u      = new_r_Conv(block, count, mode_Iu);
	ir_node *addv         = new_r_Const_long(irg, mode_Iu, add_size);
	ir_node *add1         = new_r_Add(block, count_u, addv, mode_Iu);
	ir_node *elsizev      = new_r_Const_long(irg, mode_Iu, element_size);

	ir_node *size         = new_r_Mul(block, add1, elsizev, mode_Iu);
	unsigned addr_delta   = add_size * element_size;

	symconst_symbol calloc_sym;
	calloc_sym.entity_p   = calloc_entity;
	ir_node *callee       = new_r_SymConst(irg, mode_P, calloc_sym, symconst_addr_ent);

	ir_node *one          = new_r_Const_long(irg, mode_Iu, 1);
	ir_node *in[2]        = { one, size };
	ir_type *call_type    = get_entity_type(calloc_entity);
	ir_node *call         = new_r_Call(block, cur_mem, callee, 2, in, call_type);
	cur_mem               = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node *ress         = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node *res          = new_r_Proj(ress, mode_P, 0);

	/* write length of array */
	ir_node *len_value    = new_r_Conv(block, count, default_arraylength_mode);

	ir_node *len_delta    = new_r_Const_long(irg, mode_P, (int)addr_delta-4); //FIXME: replace magic num
	ir_node *len_addr     = new_r_Add(block, res, len_delta, mode_P);
	ir_node *store        = new_r_Store(block, cur_mem, len_addr, len_value, cons_none);
	cur_mem               = new_r_Proj(store, mode_M, pn_Store_M);

	if (addr_delta > 0) {
		ir_node *delta = new_r_Const_long(irg, mode_P, (int)addr_delta);
		res = new_r_Add(block, res, delta, mode_P);
	}

	*mem = cur_mem;
	return res;
}

static ir_node *default_get_arraylength(ir_node* objptr, ir_graph *irg, ir_node *block, ir_node **mem)
{
	/* calculate address of arraylength field */
	int       length_len  = get_mode_size_bytes(default_arraylength_mode);
	ir_node  *cnst        = new_r_Const_long(irg, mode_P, -length_len);
	ir_node  *length_addr = new_r_Add(block, objptr, cnst, mode_P);

	ir_node  *cur_mem     = *mem;
	ir_node  *load        = new_r_Load(block, cur_mem, length_addr, default_arraylength_mode, cons_none);
	cur_mem               = new_r_Proj(load, mode_M, pn_Load_M);
	ir_node  *len         = new_r_Proj(load, default_arraylength_mode, pn_Load_res);
	*mem = cur_mem;
	return len;
}

void dmemory_init(void)
{
	unsigned opcode = get_next_ir_opcode();
	op_Arraylength = new_ir_op(opcode, "Arraylength", op_pin_state_floats, irop_flag_uses_memory, oparity_unary, 0, 0, &dmemory_node_op_ops);

	ir_type *type_reference = new_type_primitive(mode_P);
	ir_type *type_int       = new_type_primitive(mode_Is);
	ir_type *type_size_t    = new_type_primitive(mode_Iu);

	ir_type *calloc_type    = new_type_method(2, 1);

	set_method_param_type(calloc_type, 0, type_size_t);
	set_method_param_type(calloc_type, 1, type_size_t);
	set_method_res_type(calloc_type, 0, type_reference);
	set_method_additional_properties(calloc_type, mtp_property_malloc);

	ir_type *glob           = get_glob_type();
	ident   *calloc_id      = new_id_from_str("calloc");
	calloc_entity = new_entity(glob, calloc_id, calloc_type);
	set_entity_visibility(calloc_entity, ir_visibility_external);

	ir_type *arraylength_type = new_type_method(1, 1);
	set_method_param_type(arraylength_type, 0, type_reference);
	set_method_res_type(arraylength_type, 0, type_int);
	set_method_additional_properties(arraylength_type, mtp_property_pure);

	default_arraylength_mode = mode_Is;

	dmemory_model.alloc_object    = default_alloc_object;
	dmemory_model.alloc_array     = default_alloc_array;
	dmemory_model.get_arraylength = default_get_arraylength;
}

void dmemory_lower_Alloc(ir_node *node)
{
	assert(is_Alloc(node));

	if (get_Alloc_where(node) != heap_alloc)
		return;

	ir_graph *irg     = get_irn_irg(node);
	ir_type  *type    = get_Alloc_type(node);
	ir_node  *count   = get_Alloc_count(node);
	ir_node  *res     = NULL;
	ir_node  *cur_mem = get_Alloc_mem(node);
	ir_node  *block   = get_nodes_block(node);

	if (is_Class_type(type)) {
		res = (*dmemory_model.alloc_object)(type, irg, block, &cur_mem);
		ddispatch_prepare_new_instance(type, res, irg, block, &cur_mem);
	} else if (is_Array_type(type)) {
		ir_type *eltype  = get_array_element_type(type);
		res = (*dmemory_model.alloc_array)(eltype, count, irg, block, &cur_mem);
	} else {
		assert (0);
	}

	turn_into_tuple(node, pn_Alloc_max);
	set_irn_n(node, pn_Alloc_M, cur_mem);
	set_irn_n(node, pn_Alloc_res, res);
}

void dmemory_lower_Arraylength(ir_node *arraylength)
{
	ir_node  *array_ref = get_Arraylength_arrayref(arraylength);
	ir_node  *block     = get_nodes_block(arraylength);
	ir_graph *irg       = get_irn_irg(block);
	ir_node  *cur_mem   = get_Arraylength_mem(arraylength);
	ir_node  *len       = (*dmemory_model.get_arraylength)(array_ref, irg, block, &cur_mem);

	turn_into_tuple(arraylength, pn_Arraylength_max);
	set_irn_n(arraylength, pn_Arraylength_M, cur_mem);
	set_irn_n(arraylength, pn_Arraylength_res, len);
}

void dmemory_set_allocation_methods(alloc_object_t ao_func, alloc_array_t aa_func, get_arraylength_t ga_func)
{
	assert (ao_func);
	assert (aa_func);
	assert (ga_func);

	dmemory_model.alloc_object    = ao_func;
	dmemory_model.alloc_array     = aa_func;
	dmemory_model.get_arraylength = ga_func;
}
