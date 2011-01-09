#include "config.h"

#include "liboo/rtti.h"

#include <assert.h>
#include "adt/error.h"

static ir_op *op_InstanceOf;

enum {
	rtti_pos_InstanceOf_mem = 0,
	rtti_pos_InstanceOf_objptr = 1
};

typedef struct {
	ir_type *classtype;
} rtti_InstanceOf_attr_t;

static void dump_node(FILE *f, ir_node *irn, dump_reason_t reason)
{
	assert (is_InstanceOf(irn));
	switch (reason) {
	case dump_node_opcode_txt:
		fputs(get_op_name(get_irn_op(irn)), f);
		break;
	case dump_node_mode_txt:
		break;
	case dump_node_nodeattr_txt: {
		rtti_InstanceOf_attr_t *attr = (rtti_InstanceOf_attr_t*) get_irn_generic_attr(irn);
		fprintf(f, "%s ", get_class_name(attr->classtype));
		break;
	}
	case dump_node_info_txt: {
		rtti_InstanceOf_attr_t *attr = (rtti_InstanceOf_attr_t*) get_irn_generic_attr(irn);
		fprintf(f, "type: %s ", get_class_name(attr->classtype));
		break;
	}
	default:
		break;
	}
}

/* XXX: might be needed to define a function that extracts the type from an InstanceOf node's attributes
 * -> get_irn_attr_type */
static const ir_op_ops rtti_node_op_ops = {
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
	assert (is_Class_type(classtype));

	ir_node  *block = get_nodes_block(objptr);
	ir_graph *irg   = get_irn_irg(block);

	ir_node  *in[2];
	in[rtti_pos_InstanceOf_mem]    = mem;
	in[rtti_pos_InstanceOf_objptr] = objptr;

	ir_node *res = new_ir_node(NULL, irg, block, op_InstanceOf, mode_T, 2,	in);
	rtti_InstanceOf_attr_t *attr = (rtti_InstanceOf_attr_t*) get_irn_generic_attr(res);
	attr->classtype = classtype;

	return res;
}

ir_node *get_InstanceOf_mem(const ir_node *node)
{
	assert (is_InstanceOf(node));
	return get_irn_n(node, rtti_pos_InstanceOf_mem);
}

ir_node *get_InstanceOf_objptr(const ir_node *node)
{
	assert (is_InstanceOf(node));
	return get_irn_n(node, rtti_pos_InstanceOf_objptr);
}

ir_type *get_InstanceOf_type(const ir_node *node)
{
	assert (is_InstanceOf(node));
	rtti_InstanceOf_attr_t *attr = (rtti_InstanceOf_attr_t*) get_irn_generic_attr_const(node);
	return attr->classtype;
}

bool is_InstanceOf(const ir_node *node)
{
	return get_irn_op(node) == op_InstanceOf;
}

static struct {
	construct_runtime_typeinfo_t construct_runtime_typeinfo;
	construct_instanceof_t       construct_instanceof;
} rtti_model;

static void default_construct_runtime_typeinfo(ir_type *klass)
{
	(void) klass;
	// FIXME: NYI. This no-op instead of panic, because the RTTI not needed yet in liboo.
}

static ir_node *default_construct_instanceof(ir_node *objptr, ir_type *klass, ir_graph *irg, ir_node *block, ir_node **mem)
{
	(void) objptr; (void) klass; (void) irg; (void) block; (void) mem;
	panic("Default InstanceOf implementation NYI");
}

void rtti_init()
{
	unsigned opcode = get_next_ir_opcode();
	op_InstanceOf = new_ir_op(opcode, "InstanceOf", op_pin_state_floats, irop_flag_uses_memory, oparity_unary, 0, sizeof(rtti_InstanceOf_attr_t), &rtti_node_op_ops);

	rtti_model.construct_runtime_typeinfo = default_construct_runtime_typeinfo;
	rtti_model.construct_instanceof = default_construct_instanceof;
}

void rtti_construct_runtime_typeinfo(ir_type *klass)
{
	(*rtti_model.construct_runtime_typeinfo)(klass);
}

void rtti_lower_InstanceOf(ir_node *instanceof)
{
	ir_node  *objptr  = get_InstanceOf_objptr(instanceof);
	ir_type  *type    = get_InstanceOf_type(instanceof);
	ir_node  *block   = get_nodes_block(instanceof);
	ir_graph *irg     = get_irn_irg(instanceof);
	ir_node  *cur_mem = get_InstanceOf_mem(instanceof);
	ir_node  *res     = (*rtti_model.construct_instanceof)(objptr, type, irg, block, &cur_mem);
	ir_node  *zero    = new_r_Const_long(irg, mode_Is, 0);
	ir_node  *cmp     = new_r_Cmp(block, res, zero);
	ir_node  *proj    = new_r_Proj(cmp, mode_b, pn_Cmp_Ne);

	turn_into_tuple(instanceof, pn_InstanceOf_max);
	set_irn_n(instanceof, pn_InstanceOf_M, cur_mem);
	set_irn_n(instanceof, pn_InstanceOf_res, proj);
}

void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func)
{
	assert (func);
	rtti_model.construct_runtime_typeinfo = func;
}

void rtti_set_instanceof_constructor(construct_instanceof_t func)
{
	assert (func);
	rtti_model.construct_instanceof = func;
}
