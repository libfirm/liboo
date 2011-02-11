#ifndef OO_RTTI_H
#define OO_RTTI_H

#include <libfirm/firm.h>

typedef void     (*construct_runtime_typeinfo_t) (ir_type *klass);
typedef ir_node *(*construct_instanceof_t)       (ir_node *objptr, ir_type *klass, ir_graph *irg, ir_node *block, ir_node **mem);

void     rtti_default_construct_runtime_typeinfo(ir_type *klass);
ir_node *rtti_default_construct_instanceof(ir_node *objptr, ir_type *klass, ir_graph *irg, ir_node *block, ir_node **mem);

void rtti_init(void);
void rtti_deinit(void);
void rtti_construct_runtime_typeinfo(ir_type *klass);
void rtti_lower_InstanceOf(ir_node *instanceof);
void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func);
void rtti_set_instanceof_constructor(construct_instanceof_t func);

ir_entity *rtti_emit_string_const(const char *bytes);

#endif
