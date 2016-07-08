#include "liboo/eh.h"
#include "liboo/nodes.h"
#include "adt/error.h"
#include "adt/obst.h"
#include "libfirm/adt/pmap.h"

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

struct _method_info;
typedef struct _method_info method_info_t;
struct _method_info {
	lpad_t *top;
};

static struct obstack mem_obst;
static pmap *method_infos = NULL;
static method_info_t *obst_bottom = NULL;

static method_info_t *get_method_info(ir_graph *irg)
{
	method_info_t *info = pmap_get(method_info_t, method_infos, irg);
	assert(info || !"no info for method found! Call eh_start_r_method before!");
	return info;
}

static ir_entity *exception_object_entity;
static ir_entity *throw_entity;

#define SAVE_PUSH(type, name) type _saved_##name = get_##name();
#define SAVE_PUSH_SET(type, name, to) SAVE_PUSH(type, name); set_##name(to);
#define SAVE_POP(name) set_##name(_saved_##name);
#define NODE_PUSH(name) SAVE_PUSH(ir_node*, name)
#define NODE_PUSH_SET(name, to) SAVE_PUSH_SET(ir_node*, name, to);
#define NODE_POP(name) SAVE_POP(name);

ir_node *eh_get_exception_object(void)
{
	ir_node *const cur_mem  = get_store();
	ir_node *const ex_addr  = new_Address(exception_object_entity);
	ir_type *const void_ptr = new_type_pointer(new_type_primitive(mode_ANY));
	ir_node *const ex_load  = new_Load(cur_mem, ex_addr, mode_P, void_ptr, cons_none);
	ir_node *const new_mem  = new_Proj(ex_load, mode_M, pn_Load_M);
	ir_node *const ex_obj   = new_Proj(ex_load, mode_P, pn_Load_res);
	set_store(new_mem);
	return ex_obj;
}

static void store_exception_object(ir_node *exo_ptr)
{
	ir_node *const cur_mem  = get_store();
	ir_node *const ex_addr  = new_Address(exception_object_entity);
	ir_type *const void_ptr = new_type_pointer(new_type_primitive(mode_ANY));
	ir_node *const ex_store = new_Store(cur_mem, ex_addr, exo_ptr, void_ptr, cons_none);
	ir_node *const new_mem  = new_Proj(ex_store, mode_M, pn_Store_M);
	set_store(new_mem);
}

void eh_init(void)
{
	obstack_init(&mem_obst);
	method_infos = pmap_create();

	ir_type *type_reference = new_type_primitive(mode_P);
	exception_object_entity = new_entity(get_glob_type(), new_id_from_str("__oo_rt_exception_object__"), type_reference);
	set_entity_visibility(exception_object_entity, ir_visibility_external);
	set_entity_initializer(exception_object_entity, get_initializer_null());

	ir_type *throw_type = new_type_method(1, 0);
	set_method_param_type(throw_type, 0, type_reference);
	throw_entity = new_entity(get_glob_type(), new_id_from_str("firm_personality"), throw_type);
	set_entity_visibility(throw_entity, ir_visibility_external);
}

void eh_deinit(void)
{
	obstack_free(&mem_obst, NULL);
	pmap_destroy(method_infos);
}

static void add_handler_header_block_pred(lpad_t *lpad, ir_node *pred)
{
	add_immBlock_pred(lpad->handler_header_block, pred);
	lpad->used = true;
}

void eh_start_method(void)
{
	ir_graph *irg = get_current_ir_graph();
	assert (irg_is_constrained(irg, IR_GRAPH_CONSTRAINT_CONSTRUCTION));
	assert (!pmap_contains(method_infos, irg) || !"Method already has an info entry!");

	method_info_t *new_info = (method_info_t*) obstack_alloc(&mem_obst, sizeof(*new_info));
	new_info->top = NULL;
	pmap_insert(method_infos, irg, new_info);

	if(!obst_bottom) {
		obst_bottom = new_info;
	}
	eh_new_lpad();
}

void eh_new_lpad(void)
{
	ir_graph *irg = get_current_ir_graph();
	method_info_t *info = get_method_info(irg);
	lpad_t *new_pad = (lpad_t*) obstack_alloc(&mem_obst, sizeof(*new_pad));
	ir_node *block = new_r_immBlock(irg);
	new_pad->handler_header_block = block;
	new_pad->cur_block            = block;
	new_pad->used                 = false;
	new_pad->prev                 = info->top;

	NODE_PUSH_SET(cur_block, new_pad->cur_block);
	new_pad->exception_object     = eh_get_exception_object();
	NODE_POP(cur_block);

	info->top = new_pad;
}

void eh_add_handler(ir_type *catch_type, ir_node *catch_block)
{
	ir_graph *irg = get_current_ir_graph();
	method_info_t* info = get_method_info(irg);
	lpad_t* top = info->top;
	assert (top->prev); //e.g., not the default handler
	assert (top->cur_block && "Cannot add handler after an catch all was registered");

	NODE_PUSH_SET(cur_block, top->cur_block);

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

	NODE_POP(cur_block);
}

ir_node *eh_new_Call(ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* type)
{
	ir_graph *irg = get_current_ir_graph();
	method_info_t* info = get_method_info(irg);
	lpad_t* top = info->top;

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

	add_handler_header_block_pred(top, proj_except);

	ir_node *proj_regular = new_Proj(call, mode_X, pn_Call_X_regular);
	ir_node *new_block    = new_immBlock();
	add_immBlock_pred(new_block, proj_regular);
	mature_immBlock(new_block);
	set_cur_block(new_block);

	return call;
}

void eh_throw(ir_node *exo_ptr)
{
	ir_graph *irg = get_current_ir_graph();
	method_info_t* info = get_method_info(irg);
	lpad_t* top = info->top;

	store_exception_object(exo_ptr);

	ir_node *throw = new_Jmp();

	add_handler_header_block_pred(top, throw);
	set_cur_block(NULL);
}

void eh_pop_lpad(void)
{
	ir_graph *irg = get_current_ir_graph();
	method_info_t* info = get_method_info(irg);
	lpad_t* top = info->top;

	mature_immBlock(top->handler_header_block);

	//assert (top->used && "No exception is ever thrown");

	lpad_t *prev         = top->prev;
	assert (prev);

	if (top->cur_block) {
		// the popped lpad didn't have a catch all handler and therefore is still "open".
		NODE_PUSH_SET(cur_block, top->cur_block);

		ir_node *jmp = new_Jmp();
		add_handler_header_block_pred(prev, jmp);

		NODE_POP(cur_block);
	}

	info->top = prev;
}

void eh_end_method(void)
{
	ir_graph *irg = get_current_ir_graph();
	method_info_t* info = get_method_info(irg);
	lpad_t* top = info->top;

	assert (! top->prev); // the explicit stuff is gone, we have the default handler

	if (top->used) {
		mature_immBlock(top->handler_header_block);

		assert (top->cur_block); // would fail if front end adds an catch all handler to the default handler

		NODE_PUSH_SET(cur_block, top->cur_block);
		ir_node *cur_mem     = get_store();
		ir_node *raise       = new_Raise(cur_mem, top->exception_object);
		ir_node *proj        = new_Proj(raise, mode_X, pn_Raise_X);
		cur_mem              = new_Proj(raise, mode_M, pn_Raise_M);
		set_store(cur_mem);

		ir_node *end_block   = get_irg_end_block(irg);
		add_immBlock_pred(end_block, proj);

		NODE_POP(cur_block);
	}

	if(info == obst_bottom) {
		obstack_free(&mem_obst, info);
		obst_bottom = NULL;
	}

	//pmap_remove() ??????
}

void eh_lower_Raise(ir_node *raise, ir_node *proj)
{
	assert (is_Raise(raise) && is_Proj(proj));

	ir_graph *const irg     = get_irn_irg(raise);
	ir_node  *const block   = get_nodes_block(raise);

	/* Call "firm_personality(exception_object)" */
	ir_node  *const ex_obj  = get_Raise_exo_ptr(raise);
	ir_node  *const cur_mem = get_Raise_mem(raise);
	ir_node  *const throw_entity_addr  = new_r_Address(irg, throw_entity);
	ir_node  *const in[1]   = { ex_obj };
	ir_node  *const throw   = new_r_Call(block, cur_mem, throw_entity_addr, 1, in, get_entity_type(throw_entity));
	ir_set_needs_reloaded_callee_saves(throw, true);

	exchange(raise, throw);
	ir_set_throws_exception(throw, 1);
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

#define GRAPH_PUSH								SAVE_PUSH_SET(ir_graph*, current_ir_graph, irg);
#define GRAPH_POP								SAVE_POP(current_ir_graph);
#define GRAPH_SAVED_DO(method, ...)				GRAPH_PUSH; method(__VA_ARGS__); GRAPH_POP;
#define GRAPH_SAVED_DO_RET_NODE(method, ...)	GRAPH_PUSH; ir_node *const ret = method(__VA_ARGS__); GRAPH_POP; return ret;

void eh_start_r_method(ir_graph *irg)
{
	GRAPH_SAVED_DO(eh_start_method,);
}

void eh_end_r_method(ir_graph *irg)
{
	GRAPH_SAVED_DO(eh_end_method,);
}

void eh_new_r_lpad(ir_graph *irg)
{
	GRAPH_SAVED_DO(eh_new_lpad,);
}

void eh_pop_r_lpad(ir_graph *irg)
{
	GRAPH_SAVED_DO(eh_pop_lpad,);
}

void eh_add_r_handler(ir_graph *irg, ir_type *catch_type, ir_node *block)
{
	GRAPH_SAVED_DO(eh_add_handler, catch_type, block);
}

ir_node *eh_get_r_exception_object(ir_graph *irg)
{
	GRAPH_SAVED_DO_RET_NODE(eh_get_exception_object,);
}

ir_node *eh_new_r_Call(ir_graph *irg, ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* catch_type)
{
	GRAPH_SAVED_DO_RET_NODE(eh_new_Call, irn_ptr, arity, in, catch_type);
}

void eh_r_throw(ir_graph *irg, ir_node *exo_ptr)
{
	GRAPH_SAVED_DO(eh_throw, exo_ptr);
}
