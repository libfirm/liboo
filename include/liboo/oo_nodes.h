#ifndef OO_NODES_H
#define OO_NODES_H

#include <stdbool.h>
#include <libfirm/firm.h>

enum {
	pn_InstanceOf_M = 0,
	pn_InstanceOf_res,
	pn_InstanceOf_max
};

ir_node *new_r_InstanceOf(ir_node *block, ir_node *mem, ir_node *objptr, ir_type *classtype);
ir_node *new_InstanceOf(ir_node *mem, ir_node *objptr, ir_type *classtype);
ir_node *get_InstanceOf_mem(const ir_node *node);
void     set_InstanceOf_mem(ir_node *node, ir_node *mem);
ir_node *get_InstanceOf_objptr(const ir_node *node);
void     set_InstanceOf_objptr(ir_node *node, ir_node *objptr);
ir_type *get_InstanceOf_type(const ir_node *node);
void     set_InstanceOf_type(ir_node *node, ir_type *type);
bool     is_InstanceOf(const ir_node *node);

enum {
	pn_Arraylength_M = 0,
	pn_Arraylength_res,
	pn_Arraylength_max
};

ir_node *new_r_Arraylength(ir_node *block, ir_node *mem, ir_node* arrayref);
ir_node *new_Arraylength(ir_node *mem, ir_node* arrayref);
ir_node *get_Arraylength_mem(const ir_node *node);
void     set_Arraylength_mem(ir_node *node, ir_node *mem);
ir_node *get_Arraylength_arrayref(const ir_node *node);
void     set_Arraylength_arrayref(ir_node *node, ir_node *arrayref);
bool     is_Arraylength(const ir_node *node);

void     oo_nodes_init(void);

#endif
