#ifndef OO_EH_H
#define OO_EH_H

#include <libfirm/firm.h>

void eh_init(void);
void eh_deinit(void);

void eh_start_r_method(ir_graph *irg);
void eh_new_r_lpad(ir_graph *irg);
void eh_add_r_handler(ir_graph *irg, ir_type *catch_type, ir_node *block);
void eh_pop_r_lpad(ir_graph *irg);
void eh_end_r_method(ir_graph *irg);

ir_node *eh_get_r_exception_object(ir_graph *irg);
ir_node *eh_new_r_Call(ir_graph *irg, ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* catch_type);
void     eh_r_throw(ir_graph *irg, ir_node *exo_ptr);

void eh_start_method(void);
void eh_new_lpad(void);
void eh_add_handler(ir_type *catch_type, ir_node *block);
void eh_pop_lpad(void);
void eh_end_method(void);

ir_node *eh_get_exception_object(void);
ir_node *eh_new_Call(ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* catch_type);
void     eh_throw(ir_node *exo_ptr);

void eh_lower_Raise(ir_node *raise, ir_node *proj);

#endif
