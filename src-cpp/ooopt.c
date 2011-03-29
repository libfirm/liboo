
#include "liboo/ooopt.h"
#include "liboo/oo.h"
#include <libfirm/rta.h>

#include <stdbool.h>
#include <assert.h>

static ir_type *get_alive_subclass(ir_type *klass)
{
	assert (is_Class_type(klass));
	if (oo_get_class_is_extern(klass))
		return NULL; // we cannot be sure that we know all existing subclasses

	// FIXME: should check the existance of native methods

	ir_type *alive_class = NULL;
	size_t   alive_count = 0;
	if (rta_is_alive_class(klass)) {
		alive_class = klass;
		alive_count++;
	}

	if (! oo_get_class_is_final(klass)) {
		ir_type *iter = get_class_trans_subtype_first(klass);
		while (iter) {
			if (rta_is_alive_class(iter)) {
				alive_class = iter;
				alive_count++;
			}
			iter = get_class_trans_subtype_next(klass);
		}
	}

	if (alive_count == 1)
		return alive_class;
	else
		return NULL;
}

static ir_type *get_Sel_or_SymConst_type(ir_node *sos)
{
	assert (is_Sel(sos) || is_SymConst_addr_ent(sos));

	ir_entity *entity = get_irn_entity_attr(sos);
	ir_type   *type   = get_entity_type(entity);

	if (is_Method_type(type)) {
		size_t n_ress = get_method_n_ress(type);
		if (n_ress == 0)
			return NULL;
		assert (n_ress == 1);
		type = get_method_res_type(type, 0);
	}

	if (is_Pointer_type(type))
		type = get_pointer_points_to_type(type);

	if (is_Class_type(type))
		return type;

	return NULL;
}

static void infer_typeinfo_walker(ir_node *irn, void *env)
{
	bool *changed = (bool*) env;

	// A node's type needs only to be calculated once.
	if (get_irn_typeinfo_type(irn) != initial_type)
		return;

	if (is_Alloc(irn)) {
		// this one is easy, we know the exact dynamic type.
		ir_type *type = get_Alloc_type(irn);
		if (! is_Class_type(type))
			return;

		set_irn_typeinfo_type(irn, type);
		*changed = true;
	}
	else if (is_Sel(irn) || is_SymConst_addr_ent(irn)) {
		// the type we determine here is the one of the entity we select or reference.
		// the transform_Sel method below will use the type incoming on the Sel_ptr input.
		ir_type *type = get_Sel_or_SymConst_type(irn);
		if (! type)
			return;

		ir_type *one_alive = get_alive_subclass(type);
		if (! one_alive)
			return;

		set_irn_typeinfo_type(irn, one_alive);
		*changed = true;
	}
	else if (is_Call(irn)) {
		// the dynamic type of the call result is the return type of the called entity.
		ir_node *call_pred = get_Call_ptr(irn);
		ir_type *pred_type = get_irn_typeinfo_type(call_pred);
		if (pred_type == initial_type)
			return;

		set_irn_typeinfo_type(irn, pred_type);
		*changed = true;
	}
	else if (is_Load(irn)) {
		// the dynamic type of the Load result is the type of the loaded entity.
		ir_node *load_pred = get_Load_ptr(irn);

		if (! is_Sel(load_pred) && !is_SymConst_addr_ent(load_pred))
			return;

		ir_type *pred_type = get_irn_typeinfo_type(load_pred);
		if (pred_type == initial_type)
			return;
		set_irn_typeinfo_type(irn, pred_type);
		*changed = true;
	}
	else if (is_Proj(irn)) {
		// Types have to be propagated through Proj nodes (XXX: and also through Cast and Confirm
		ir_mode *pmode = get_irn_mode(irn);
		if (pmode != mode_P)
			return;

		ir_node *proj_pred = get_Proj_pred(irn);
		if (is_Proj(proj_pred) && get_irn_mode(proj_pred) == mode_T && get_Proj_proj(proj_pred) == pn_Call_T_result && is_Call(get_Proj_pred(proj_pred)))
			proj_pred = get_Proj_pred(proj_pred); // skip the result tuple

		ir_type *pred_type = get_irn_typeinfo_type(proj_pred);
		if (pred_type == initial_type)
			return;

		set_irn_typeinfo_type(irn, pred_type);
		*changed = true;
	}
	else if (is_Phi(irn)) {
		// Phi nodes are a special case because the incoming type information must be merged
		// A Phi node's type is unknown until all inputs are known to be the same dynamic type.
		ir_mode *pmode = get_irn_mode(irn);
		if (pmode != mode_P)
			return;

		int phi_preds = get_Phi_n_preds(irn);
		ir_type *last = NULL;
		for (int p = 0; p < phi_preds; p++) {
			ir_node *pred = get_Phi_pred(irn, p);
			ir_type *pred_type = get_irn_typeinfo_type(pred);
			if (pred_type == initial_type)
				return;
			if (p && last != pred_type)
				return;

			last = pred_type;
		}
		set_irn_typeinfo_type(irn, last);
	}
}

/*
 * Transform Sel[method] to SymC[method] if possible.
 * (see opt_polymorphy in libfirm)
 */
static ir_node *transform_node_Sel2(ir_node *node)
{
	ir_node   *new_node;
	ir_entity *ent = get_Sel_entity(node);

	if (get_irp_phase_state() == phase_building) return node;

	if (!get_opt_dyn_meth_dispatch())
		return node;

	if (!is_Method_type(get_entity_type(ent)))
		return node;

	ddispatch_binding bind = oo_get_entity_binding(ent);
	assert (bind != bind_unknown);

	if (bind == bind_static)
		return node;

	/* If we know the dynamic type, we can replace the Sel by a constant. */
	ir_node *ptr    = get_Sel_ptr(node);      /* The address we select from. */
	ir_type *dyn_tp = get_irn_typeinfo_type(ptr);

	if (dyn_tp != initial_type) {
		ir_entity *called_ent;

		/* We know which method will be called, no dispatch necessary. */
		called_ent = resolve_ent_polymorphy(dyn_tp, ent);
		assert (! oo_get_method_is_abstract(called_ent));
		assert (! oo_get_class_is_interface(get_entity_owner(called_ent)));

		new_node = copy_const_value(get_irn_dbg_info(node), get_atomic_ent_value(called_ent), get_nodes_block(node));

		return new_node;
	}

	return node;
}

void oo_devirtualize_local(ir_graph *irg)
{
	//dump_ir_graph(irg, "--before");

	rta_init();

	init_irtypeinfo();
	set_irg_typeinfo_state(irg, ir_typeinfo_consistent);

	compute_inh_transitive_closure();

	bool changed;
	do {
		changed = false;
		irg_walk_graph(irg, infer_typeinfo_walker, NULL, (void*)&changed);
	} while (changed);

	//dump_ir_graph(irg, "--typeinfo");

	set_opt_dyn_meth_dispatch(1);

	transform_node_func oldfunc = get_op_ops(get_op_Sel())->transform_node;
	((ir_op_ops*)get_op_ops(get_op_Sel()))->transform_node = transform_node_Sel2;
	local_optimize_graph(irg);

	((ir_op_ops*)get_op_ops(get_op_Sel()))->transform_node = oldfunc; // restore

	free_irtypeinfo();

	rta_cleanup();

	//dump_ir_graph(irg, "--devirtualize-local");
}
