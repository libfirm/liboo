#include "liboo/eh.h"
#include "liboo/nodes.h"
#include "adt/error.h"
#include "adt/obst.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef LIBOO_EXCEPTION_SUPPORT

struct _lpad_t;
typedef struct _lpad_t lpad_t;
struct _lpad_t {
	ir_node *handler_header_block;
	ir_node *cur_block;
	ir_node *exception_object;
	bool     used;
	lpad_t  *prev;
};

static struct obstack lpads;
static lpad_t *top;

static ir_entity *exception_object_entity;
static ir_entity *throw_entity;

ir_node *eh_get_exception_object(void)
{
	ir_node *cur_mem = get_store();

	ir_node *ex_symc = new_Address(exception_object_entity);
	ir_node *ex_load = new_Load(cur_mem, ex_symc, mode_P, cons_none);
	cur_mem          = new_Proj(ex_load, mode_M, pn_Load_M);
	ir_node *ex_obj  = new_Proj(ex_load, mode_P, pn_Load_res);

	set_store(cur_mem);
	return ex_obj;
}

static void store_exception_object(ir_node *exo_ptr)
{
	ir_node *cur_mem  = get_store();

	ir_node *ex_symc  = new_Address(exception_object_entity);
	ir_node *ex_store = new_Store(cur_mem, ex_symc, exo_ptr, cons_none);
	cur_mem           = new_Proj(ex_store, mode_M, pn_Store_M);

	set_store(cur_mem);
}

void eh_init(void)
{
	obstack_init(&lpads);
	ir_type *type_reference = new_type_primitive(mode_P);
	exception_object_entity = new_entity(get_glob_type(), new_id_from_str("__oo_rt_exception_object__"), type_reference);
	set_entity_initializer(exception_object_entity, get_initializer_null());

	ir_type *throw_type = new_type_method(1, 0);
	set_method_param_type(throw_type, 0, type_reference);
	throw_entity = new_entity(get_glob_type(), new_id_from_str("firm_personality"), throw_type);
	set_entity_visibility(throw_entity, ir_visibility_external);

	top = NULL;
}

void eh_deinit(void)
{
	obstack_free(&lpads, NULL);
}

void eh_start_method(void)
{
	assert (irg_is_constrained(get_current_ir_graph(), IR_GRAPH_CONSTRAINT_CONSTRUCTION));
	assert (obstack_object_size(&lpads) == 0);
	eh_new_lpad();
}

void eh_new_lpad(void)
{
	lpad_t *new_pad = (lpad_t*) obstack_alloc(&lpads, sizeof(lpad_t));
	new_pad->handler_header_block = new_immBlock();
	new_pad->cur_block            = new_pad->handler_header_block;
	new_pad->exception_object     = NULL;
	new_pad->used                 = false;
	new_pad->prev                 = top;

	top = new_pad;

	ir_node *saved_block  = get_cur_block();
	set_cur_block(new_pad->cur_block);

	top->exception_object = eh_get_exception_object();

	set_cur_block(saved_block);
}

void eh_add_handler(ir_type *catch_type, ir_node *catch_block)
{
	assert (top->prev); //e.g., not the default handler
	assert (top->cur_block && "Cannot add handler after an catch all was registered");

	ir_node *saved_block = get_cur_block();
	set_cur_block(top->cur_block);

	if (catch_type) {
		ir_node *cur_mem     = get_store();
		ir_node *instanceof  = new_InstanceOf(cur_mem, top->exception_object, catch_type);
		cur_mem              = new_Proj(instanceof, mode_M, pn_InstanceOf_M);
		ir_node *result      = new_Proj(instanceof, mode_b, pn_InstanceOf_res);
		ir_node *cond        = new_Cond(result);

		ir_node *proj_match  = new_Proj(cond, mode_X, pn_Cond_true);
		add_immBlock_pred(catch_block, proj_match);
		// will be matured elsewhere

		ir_node *proj_go_on  = new_Proj(cond, mode_X, pn_Cond_false);
		ir_node *new_block   = new_immBlock();
		add_immBlock_pred(new_block, proj_go_on);
		mature_immBlock(new_block);
		top->cur_block = new_block;
		set_store(cur_mem);
	} else {
		ir_node *jmp = new_Jmp();
		add_immBlock_pred(catch_block, jmp);
		top->cur_block = NULL;
	}

	set_cur_block(saved_block);
}

ir_node *eh_new_Call(ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* type)
{
	ir_node *jmp          = new_Jmp();
	ir_node *call_block   = new_immBlock();
	add_immBlock_pred(call_block, jmp);
	mature_immBlock(call_block);
	set_cur_block(call_block);

	ir_node *cur_mem      = get_store();
	ir_node *call         = new_Call(cur_mem, irn_ptr, arity, in, type);
	ir_set_throws_exception(call, 1);
	ir_node *proj_except  = new_Proj(call, mode_X, pn_Call_X_except);
	cur_mem               = new_Proj(call, mode_M, pn_Call_M);
	set_store(cur_mem);

	add_immBlock_pred(top->handler_header_block, proj_except);

	ir_node *proj_regular = new_Proj(call, mode_X, pn_Call_X_regular);
	ir_node *new_block    = new_immBlock();
	add_immBlock_pred(new_block, proj_regular);
	mature_immBlock(new_block);
	set_cur_block(new_block);

	top->used = true;

	return call;
}

void eh_throw(ir_node *exo_ptr)
{
	store_exception_object(exo_ptr);

	ir_node *throw = new_Jmp();

	add_immBlock_pred(top->handler_header_block, throw);
	top->used = true;
	set_cur_block(NULL);
}

void eh_pop_lpad(void)
{
	mature_immBlock(top->handler_header_block);

	//assert (top->used && "No exception is ever thrown");

	lpad_t *prev         = top->prev;
	assert (prev);

	if (top->cur_block) {
		// the popped lpad didn't have a catch all handler and therefore is still "open".
		ir_node *saved_block = get_cur_block();
		set_cur_block(top->cur_block);

		ir_node *jmp         = new_Jmp();
		add_immBlock_pred(prev->handler_header_block, jmp);

		set_cur_block(saved_block);
	}

	obstack_free(&lpads, top);
	top = prev;
}

void eh_end_method(void)
{
	assert (! top->prev); // the explicit stuff is gone, we have the default handler

	if (top->used) {
		mature_immBlock(top->handler_header_block);

		assert (top->cur_block); // would fail if front end adds an catch all handler to the default handler

		ir_node *saved_block = get_cur_block();
		set_cur_block(top->cur_block);
		ir_node *cur_mem     = get_store();
		ir_node *raise       = new_Raise(cur_mem, top->exception_object);
		ir_node *proj        = new_Proj(raise, mode_X, pn_Raise_X);
		cur_mem              = new_Proj(raise, mode_M, pn_Raise_M);
		set_store(cur_mem);

		ir_node *end_block   = get_irg_end_block(get_current_ir_graph());
		add_immBlock_pred(end_block, proj);

		set_cur_block(saved_block);
	}

	obstack_free(&lpads, top);
	top = NULL;
}

void eh_lower_Raise(ir_node *raise, ir_node *proj)
{
	assert (is_Raise(raise) && is_Proj(proj));

	ir_node  *ex_obj  = get_Raise_exo_ptr(raise);
	ir_node  *block   = get_nodes_block(raise);
	ir_graph *irg     = get_irn_irg(raise);
	ir_node  *cur_mem = get_Raise_mem(raise);

	ir_node  *c_symc  = new_r_SymConst(irg, throw_entity);
	ir_node  *in[1]   = { ex_obj };

	ir_node  *throw   = new_r_Call(block, cur_mem, c_symc, 1, in, get_entity_type(throw_entity));
	ir_set_throws_exception(throw, 1);
	exchange(raise, throw);
	set_Proj_num(proj, pn_Call_X_except);
}

#else

ir_node *eh_get_exception_object(void)
{
	panic("liboo compiled without exception support");
}

void eh_init(void)
{
}

void eh_deinit(void)
{
}

void eh_start_method(void)
{
	panic("liboo compiled without exception support");
}

void eh_new_lpad(void)
{
	panic("liboo compiled without exception support");
}

void eh_add_handler(ir_type *catch_type, ir_node *catch_block)
{
	(void)catch_type;
	(void)catch_block;
	panic("liboo compiled without exception support");
}

ir_node *eh_new_Call(ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* type)
{
	(void)irn_ptr;
	(void)arity;
	(void)in;
	(void)type;
	panic("liboo compiled without exception support");
}

void eh_throw(ir_node *exo_ptr)
{
	(void)exo_ptr;
	panic("liboo compiled without exception support");
}

void eh_pop_lpad(void)
{
	panic("liboo compiled without exception support");
}

void eh_end_method(void)
{
	panic("liboo compiled without exception support");
}

void eh_lower_Raise(ir_node *raise, ir_node *proj)
{
	(void)raise;
	(void)proj;
	panic("liboo compiled without exception support");
}
#endif
