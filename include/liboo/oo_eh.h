#ifndef OO_EH_H
#define OO_EH_H

#include <libfirm/firm.h>

void oo_eh_init(void);
void oo_eh_deinit(void);

void oo_eh_start_method(void);
void oo_eh_new_lpad(void);
void oo_eh_add_handler(ir_type *catch_type, ir_node *block);
void oo_eh_pop_lpad(void);
void oo_eh_end_method(void);

ir_node *oo_eh_get_exception_object(void);
ir_node *oo_eh_new_Call(ir_node * irn_mem, ir_node * irn_ptr, int arity, ir_node *const * in, ir_type* catch_type);

#endif
