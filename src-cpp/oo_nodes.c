#include "config.h"
#include "liboo/oo_nodes.h"

#include <stdio.h>
#include <assert.h>

static ir_op *op_InstanceOf;
static ir_op *op_Arraylength;

enum {
	pos_InstanceOf_mem = 0,
	pos_InstanceOf_objptr = 1
};

enum {
	pos_Arraylength_mem = 0,
	pos_Arraylength_arrayref = 1
};

typedef struct {
	ir_type *classtype;
} op_InstanceOf_attr_t;

static void dump_node(FILE *f, ir_node *irn, dump_reason_t reason)
{
	assert (is_InstanceOf(irn) || is_Arraylength(irn));
	switch (reason) {
	case dump_node_opcode_txt:
		fputs(get_op_name(get_irn_op(irn)), f);
		break;
	case dump_node_mode_txt:
		break;
	case dump_node_nodeattr_txt: {
		if (! is_InstanceOf(irn)) break;

		op_InstanceOf_attr_t *attr = (op_InstanceOf_attr_t*) get_irn_generic_attr(irn);
		fprintf(f, "%s ", get_class_name(attr->classtype));
		break;
	}
	case dump_node_info_txt: {
		if (! is_InstanceOf(irn)) break;

		op_InstanceOf_attr_t *attr = (op_InstanceOf_attr_t*) get_irn_generic_attr(irn);
		fprintf(f, "type: %s ", get_class_name(attr->classtype));
		break;
	}
	default:
		break;
	}
}

/* XXX: might be needed to define a function that extracts the type from an InstanceOf node's attributes
 * -> get_irn_attr_type */
static const ir_op_ops oo_nodes_op_ops = {
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

ir_node *new_InstanceOf(ir_node *mem, ir_node *objptr, ir_type *classtype)
{
	ir_node *block = get_cur_block();
	return new_r_InstanceOf(block, mem, objptr, classtype);
}

ir_node *new_r_InstanceOf(ir_node *block, ir_node *mem, ir_node *objptr, ir_type *classtype)
{
	assert (is_Class_type(classtype));

	ir_graph *irg   = get_irn_irg(block);

	ir_node  *in[2];
	in[pos_InstanceOf_mem]    = mem;
	in[pos_InstanceOf_objptr] = objptr;

	ir_node *res = new_ir_node(NULL, irg, block, op_InstanceOf, mode_T, 2,	in);
	op_InstanceOf_attr_t *attr = (op_InstanceOf_attr_t*) get_irn_generic_attr(res);
	attr->classtype = classtype;

	return res;
}

ir_node *get_InstanceOf_mem(const ir_node *node)
{
	assert (is_InstanceOf(node));
	return get_irn_n(node, pos_InstanceOf_mem);
}

void set_InstanceOf_mem(ir_node *node, ir_node *mem)
{
	assert (is_InstanceOf(node));
	set_irn_n(node, pos_InstanceOf_mem, mem);
}

ir_node *get_InstanceOf_objptr(const ir_node *node)
{
	assert (is_InstanceOf(node));
	return get_irn_n(node, pos_InstanceOf_objptr);
}

void set_InstanceOf_objptr(ir_node *node, ir_node *objptr)
{
	assert (is_InstanceOf(node));
	set_irn_n(node, pos_InstanceOf_objptr, objptr);
}

ir_type *get_InstanceOf_type(const ir_node *node)
{
	assert (is_InstanceOf(node));
	const op_InstanceOf_attr_t *attr = (const op_InstanceOf_attr_t*) get_irn_generic_attr_const(node);
	return attr->classtype;
}

void set_InstanceOf_type(ir_node *node, ir_type *type)
{
	assert (is_InstanceOf(node));
	op_InstanceOf_attr_t *attr = (op_InstanceOf_attr_t*) get_irn_generic_attr(node);
	attr->classtype = type;
}

bool is_InstanceOf(const ir_node *node)
{
	return get_irn_op(node) == op_InstanceOf;
}

ir_node *new_Arraylength(ir_node* mem, ir_node *arrayref)
{
	ir_node *block = get_cur_block();
	return new_r_Arraylength(block, mem, arrayref);
}

ir_node *new_r_Arraylength(ir_node *block, ir_node* mem, ir_node *arrayref)
{
	ir_graph *irg   = get_irn_irg(block);

	ir_node  *in[2];
	in[pos_Arraylength_mem]      = mem;
	in[pos_Arraylength_arrayref] = arrayref;

	ir_node *res = new_ir_node(NULL, irg, block, op_Arraylength, mode_T, 2,	in);

	return res;
}

ir_node *get_Arraylength_mem(const ir_node *node)
{
	assert (is_Arraylength(node));
	return get_irn_n(node, pos_Arraylength_mem);
}

void set_Arraylength_mem(ir_node *node, ir_node *mem)
{
	assert (is_Arraylength(node));
	set_irn_n(node, pos_Arraylength_mem, mem);
}

ir_node *get_Arraylength_arrayref(const ir_node *node)
{
	assert (is_Arraylength(node));
	return get_irn_n(node, pos_Arraylength_arrayref);
}

void set_Arraylength_arrayref(ir_node *node, ir_node *arrayref)
{
	assert (is_Arraylength(node));
	set_irn_n(node, pos_Arraylength_arrayref, arrayref);
}

bool is_Arraylength(const ir_node *node)
{
	return get_irn_op(node) == op_Arraylength;
}

void oo_nodes_init()
{
	unsigned opcode = get_next_ir_opcode();
	op_InstanceOf  = new_ir_op(opcode, "InstanceOf", op_pin_state_floats, irop_flag_uses_memory, oparity_unary, 0, sizeof(op_InstanceOf_attr_t), &oo_nodes_op_ops);

	opcode = get_next_ir_opcode();
	op_Arraylength = new_ir_op(opcode, "Arraylength", op_pin_state_floats, irop_flag_uses_memory, oparity_unary, 0, 0, &oo_nodes_op_ops);
}
