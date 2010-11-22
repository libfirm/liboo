#include <liboo/dmemory.h>

#include <liboo/ddispatch.h>
#include <assert.h>

static dmemory_params dmp;

void dmemory_init(dmemory_params params)
{
	dmp = params;
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
		res = (*dmp.heap_alloc_object_func)(type, irg, block, &cur_mem);
		ddispatch_prepare_new_instance(type, res, irg, block, &cur_mem);
	} else if (is_Array_type(type)) {
		ir_type *eltype  = get_array_element_type(type);
		res = (*dmp.heap_alloc_array_func)(eltype, count, irg, block, &cur_mem);
	} else {
		assert (0);
	}

	turn_into_tuple(node, pn_Alloc_max);
	set_irn_n(node, pn_Alloc_M, cur_mem);
	set_irn_n(node, pn_Alloc_res, res);
}

void dmemory_lower_arraylength(ir_node *call)
{
	ir_node  *array_ref = get_Call_param(call, 0);
	ir_node  *block     = get_nodes_block(call);
	ir_graph *irg       = get_irn_irg(block);
	ir_node  *cur_mem   = get_Call_mem(call);
	ir_node  *len       = (*dmp.arraylength_get_func)(array_ref, irg, block, &cur_mem);
	ir_node  *in[]      = { len };
	ir_node  *lent      = new_r_Tuple(block, sizeof(in)/sizeof(*in), in);

	turn_into_tuple(call, pn_Call_max);
	set_irn_n(call, pn_Call_M, cur_mem);
	set_irn_n(call, pn_Call_T_result, lent);
}
