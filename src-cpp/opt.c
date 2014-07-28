#include <assert.h>
#include <libfirm/firm_types.h>
#include <libfirm/irnode.h>
#include <libfirm/tv.h>
#include <libfirm/typerep.h>
#include <libfirm/irop.h>
#include <stdbool.h>

#include "adt/error.h"
#include "adt/util.h"
#include "liboo/nodes.h"
#include "liboo/oo.h"
#include "liboo/opt.h"

static bool is_subtype(ir_type *test, ir_type *type)
{
	if (test == type)
		return true;
	for (size_t i = 0, n = get_class_n_supertypes(test); i < n; ++i) {
		ir_type *super = get_class_supertype(test, i);
		if (is_subtype(super, type))
			return true;
	}
	return false;
}

static ir_node *transform_node_InstanceOf(ir_node *node)
{
	bool     result;
	ir_node *ptr = get_InstanceOf_ptr(node);
	// The null-pointer is not an instance of anything
	if (is_Const(ptr) && tarval_is_null(get_Const_tarval(ptr))) {
		result = false;
		goto make_tuple;
	}

	/* if we have a VptrIsSet as predecessor then we can evaluate the
	 * instanceof right away */
	if (!is_Proj(ptr))
		return node;
	ir_node *pred = get_Proj_pred(ptr);
	if (!is_VptrIsSet(pred))
		return node;

	ir_type *tested = get_InstanceOf_type(node);
	ir_type *actual = get_VptrIsSet_type(pred);
	result = is_subtype(actual, tested);

make_tuple:;
	ir_graph *irg    = get_irn_irg(node);
	ir_node  *res    = new_r_Const(irg, result ? tarval_b_true : tarval_b_false);
	ir_node *tuple_in[] = {
		[pn_InstanceOf_M]   = get_InstanceOf_mem(node),
		[pn_InstanceOf_res] = res,
	};
	ir_node *block = get_nodes_block(node);
	return new_r_Tuple(block, ARRAY_SIZE(tuple_in), tuple_in);
}

static ir_node *transform_node_MethodSel(ir_node *methodsel)
{
	ir_node   *objptr = get_MethodSel_ptr(methodsel);
	ir_entity *method = get_MethodSel_entity(methodsel);
	assert(is_method_entity(method));

	ddispatch_binding binding = oo_get_entity_binding(method);
	if (binding == bind_static)
		return methodsel;

	if (!is_Proj(objptr))
		return methodsel;
	objptr = get_Proj_pred(objptr);
	if (!is_VptrIsSet(objptr))
		return methodsel;

	ir_type   *actual        = get_VptrIsSet_type(objptr);
	ir_entity *actual_method = NULL;
	ident     *methid        = get_entity_ident(method);
	for (ir_type *clazz = actual; clazz != NULL; clazz = oo_get_class_superclass(clazz)) {
		for (size_t i = 0; i < get_class_n_members(clazz); ++i) {
			ir_entity *member = get_class_member(clazz, i);
			if (!is_method_entity(member))
				continue;
			ident *id = get_entity_ident(member);
			if (id == methid) {
				actual_method = member;
				goto make_tuple;
			}
		}
	}

	panic("Could not find method implementation");

make_tuple:;
	ir_graph *irg  = get_irn_irg(methodsel);
	dbg_info *dbgi = get_irn_dbg_info(methodsel);
	ir_node  *addr = new_rd_Address(dbgi, irg, actual_method);
	ir_node *tuple_in[] = {
		[pn_MethodSel_M]   = get_MethodSel_mem(methodsel),
		[pn_MethodSel_res] = addr,
	};
	ir_node *block = get_nodes_block(methodsel);
	return new_r_Tuple(block, ARRAY_SIZE(tuple_in), tuple_in);
}

void oo_register_opt_funcs(void)
{
	set_op_transform_node(op_InstanceOf, transform_node_InstanceOf);
	set_op_transform_node(op_MethodSel, transform_node_MethodSel);
}
