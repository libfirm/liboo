#include "liboo/oo_eh.h"
#include "liboo/oo_nodes.h"
#include "adt/obst.h"

#include <assert.h>
#include <stdio.h>

struct _lpad_t;
typedef struct _lpad_t lpad_t;
struct _lpad_t {
	ir_node *handler_header_block;
	ir_node *cur_block;
	ir_node *exception_object;
	lpad_t  *prev;
};

static struct obstack lpads;
static lpad_t *top;

static ir_entity *exception_object_entity;

ir_node *oo_eh_get_exception_object(void)
{
	symconst_symbol ex_sym;
	ex_sym.entity_p = exception_object_entity;

	ir_node *cur_mem = get_store();

	ir_node *ex_symc = new_SymConst(mode_P, ex_sym, symconst_addr_ent);
	ir_node *ex_load = new_Load(cur_mem, ex_symc, mode_P, cons_none);
	cur_mem          = new_Proj(ex_load, mode_M, pn_Load_M);
	ir_node *ex_obj  = new_Proj(ex_load, mode_P, pn_Load_res);

	set_store(cur_mem);
	return ex_obj;
}

void oo_eh_init(void)
{
	obstack_init(&lpads);
	ir_type *type_reference = new_type_primitive(mode_P);
	exception_object_entity = new_entity(get_tls_type(), new_id_from_str("__oo_exception"), type_reference);
	top = NULL;
}

void oo_eh_deinit(void)
{
	obstack_free(&lpads, NULL);
}

void oo_eh_start_method(void)
{
	assert (get_irg_phase_state(get_current_ir_graph()) == phase_building);
	assert (obstack_object_size(&lpads) == 0);
	oo_eh_new_lpad();
}

void oo_eh_new_lpad(void)
{
	lpad_t *new_pad = (lpad_t*) obstack_alloc(&lpads, sizeof(lpad_t));
	new_pad->handler_header_block = new_immBlock();
	new_pad->cur_block            = new_pad->handler_header_block;
	new_pad->exception_object     = NULL;
	new_pad->prev                 = top;

	top = new_pad;

	ir_node *saved_block  = get_cur_block();
	set_cur_block(new_pad->cur_block);

	top->exception_object = oo_eh_get_exception_object();

	set_cur_block(saved_block);
}

void oo_eh_add_handler(ir_type *catch_type, ir_node *catch_block)
{
	assert (top->prev); //e.g., not the default handler

	ir_node *saved_block = get_cur_block();
	ir_node *cur_mem     = get_store();
	set_cur_block(top->cur_block);

	ir_node *instanceof  = new_InstanceOf(cur_mem, top->exception_object, catch_type);
	cur_mem              = new_Proj(instanceof, mode_M, pn_InstanceOf_M);
	ir_node *result      = new_Proj(instanceof, mode_Is, pn_InstanceOf_res);
	ir_node *cmp         = new_Cmp(result, new_Const_long(mode_Is, 0), ir_relation_less_greater);
	ir_node *cond        = new_Cond(cmp);

	ir_node *proj_match  = new_Proj(cond, mode_X, pn_Cond_true);
	add_immBlock_pred(catch_block, proj_match);
	// will be matured elsewhere

	ir_node *proj_go_on  = new_Proj(cond, mode_X, pn_Cond_false);
	ir_node *new_block   = new_immBlock();
	add_immBlock_pred(new_block, proj_go_on);
	mature_immBlock(new_block);
	top->cur_block = new_block;

	set_store(cur_mem);
	set_cur_block(saved_block);
}

ir_node *oo_eh_new_Call(ir_node * irn_mem, ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* type)
{
	ir_node *call         = new_Call(irn_mem, irn_ptr, arity, in, type);
	ir_node *proj_except  = new_Proj(call, mode_X, pn_Call_X_except);
	add_immBlock_pred(top->handler_header_block, proj_except);

	ir_node *proj_regular = new_Proj(call, mode_X, pn_Call_X_regular);
	ir_node *new_block    = new_immBlock();
	add_immBlock_pred(new_block, proj_regular);
	mature_immBlock(new_block);
	set_cur_block(new_block);

	return call;
}

void oo_eh_pop_lpad(void)
{
	mature_immBlock(top->handler_header_block);

	lpad_t *prev         = top->prev;
	assert (prev);

	ir_node *saved_block = get_cur_block();
	set_cur_block(top->cur_block);

	ir_node *jmp         = new_Jmp();
	add_immBlock_pred(prev->handler_header_block, jmp);

	set_cur_block(saved_block);

	obstack_free(&lpads, top);
	top = prev;
}

void oo_eh_end_method(void)
{
	assert (! top->prev); // the explicit stuff is gone, we have the default handler

	mature_immBlock(top->handler_header_block);

	ir_node *saved_block = get_cur_block();
	set_cur_block(top->cur_block);
	ir_node *cur_mem     = get_store();
	ir_node *raise       = new_Raise(cur_mem, top->exception_object);
	keep_alive(raise);

	set_cur_block(saved_block);

	obstack_free(&lpads, top);
	top = NULL;
}

