#include "liboo/dmemory.h"
#include "liboo/nodes.h"

#include <assert.h>
#include "liboo/ddispatch.h"
#include "liboo/oo.h"
#include "adt/error.h"
#include "adt/util.h"

struct dmemory_model_t {
	get_arraylength_t get_arraylength;
} dmemory_model;

static ir_mode *default_arraylength_mode;

ir_node *dmemory_default_get_arraylength(dbg_info *dbgi, ir_node *block,
                                         ir_node *objptr, ir_node **mem)
{
	/* calculate address of arraylength field */
	ir_graph *irg         = get_irn_irg(block);
	int       length_len  = get_mode_size_bytes(default_arraylength_mode);
	ir_node  *cnst        = new_r_Const_long(irg, mode_P, -length_len);
	ir_node  *length_addr = new_rd_Add(dbgi, block, objptr, cnst, mode_P);

	ir_node  *cur_mem     = *mem;
	ir_node  *load        = new_rd_Load(dbgi, block, cur_mem, length_addr, default_arraylength_mode, cons_none);
	cur_mem               = new_r_Proj(load, mode_M, pn_Load_M);
	ir_node  *len         = new_r_Proj(load, default_arraylength_mode, pn_Load_res);
	*mem = cur_mem;
	return len;
}

void dmemory_init(void)
{
	default_arraylength_mode = mode_Is;
	dmemory_model.get_arraylength = dmemory_default_get_arraylength;
}

void dmemory_lower_Arraylength(ir_node *arraylength)
{
	dbg_info *dbgi      = get_irn_dbg_info(arraylength);
	ir_node  *array_ref = get_Arraylength_ptr(arraylength);
	ir_node  *block     = get_nodes_block(arraylength);
	ir_node  *cur_mem   = get_Arraylength_mem(arraylength);
	ir_node  *len       = (*dmemory_model.get_arraylength)(dbgi, block, array_ref, &cur_mem);

	ir_node *in[pn_Arraylength_max+1] = {
		[pn_Arraylength_M]   = cur_mem,
		[pn_Arraylength_res] = len
	};
	turn_into_tuple(arraylength, ARRAY_SIZE(in), in);
}

void dmemory_set_allocation_methods(get_arraylength_t ga_func)
{
	assert(ga_func);
	dmemory_model.get_arraylength = ga_func;
}
