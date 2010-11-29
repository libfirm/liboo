#include <liboo/dmemory.h>

#include <liboo/ddispatch.h>
#include <assert.h>
#include "adt/error.h"

static ir_entity *builtin_arraylength;

struct dmemory_model_t {
	alloc_object_t    alloc_object;
	alloc_array_t     alloc_array;
	get_arraylength_t get_arraylength;
} dmemory_model;

static ir_node *default_alloc_object(ir_type *type, ir_graph *irg, ir_node *block, ir_node **mem)
{
	//FIXME
	(void) type; (void) irg; (void) block; (void) mem;
	panic("default_alloc_object NYI");
}

static ir_node *default_alloc_array(ir_type *eltype, ir_node *count, ir_graph *irg, ir_node *block, ir_node **mem)
{
	//FIXME
	(void) eltype; (void) count; (void) irg; (void) block; (void) mem;
	panic("default_alloc_array NYI");
}

static ir_node *default_get_arraylength(ir_node* objptr, ir_graph *irg, ir_node *block, ir_node **mem)
{
	//FIXME
	(void) objptr; (void) irg; (void) block; (void) mem;
	panic("default_get_arraylength NYI");
}

void dmemory_init(void)
{
	ir_type* type_reference = new_type_primitive(mode_P);
	ir_type* type_int       = new_type_primitive(mode_Is);

	ir_type *arraylength_type = new_type_method(1, 1);
	set_method_param_type(arraylength_type, 0, type_reference);
	set_method_res_type(arraylength_type, 0, type_int);
	set_method_additional_properties(arraylength_type, mtp_property_pure);

	ir_type *global_type    = get_glob_type();
	ident   *arraylength_id = new_id_from_str("$builtin_arraylength");
	builtin_arraylength     = new_entity(global_type, arraylength_id, arraylength_type);
	set_entity_additional_properties(builtin_arraylength, mtp_property_intrinsic|mtp_property_private);

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

void dmemory_lower_arraylength(ir_node *call)
{
	ir_node  *array_ref = get_Call_param(call, 0);
	ir_node  *block     = get_nodes_block(call);
	ir_graph *irg       = get_irn_irg(block);
	ir_node  *cur_mem   = get_Call_mem(call);
	ir_node  *len       = (*dmemory_model.get_arraylength)(array_ref, irg, block, &cur_mem);
	ir_node  *in[]      = { len };
	ir_node  *lent      = new_r_Tuple(block, sizeof(in)/sizeof(*in), in);

	turn_into_tuple(call, pn_Call_max);
	set_irn_n(call, pn_Call_M, cur_mem);
	set_irn_n(call, pn_Call_T_result, lent);
}

ir_entity* dmemory_get_arraylength_entity(void)
{
	return builtin_arraylength;
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
