#ifndef OO_RTTI_H
#define OO_RTTI_H

#include <stdbool.h>
#include <libfirm/firm.h>

enum {
	pn_InstanceOf_M         = pn_Generic_M,
	pn_InstanceOf_res = pn_Generic_other,
	pn_InstanceOf_max
};

ir_node *new_InstanceOf(ir_node *mem, ir_node *objptr, ir_type *classtype);
ir_node *get_InstanceOf_mem(const ir_node *node);
ir_node *get_InstanceOf_objptr(const ir_node *node);
ir_type *get_InstanceOf_type(const ir_node *node);
bool     is_InstanceOf(const ir_node *node);

typedef void     (*construct_runtime_typeinfo_t) (ir_type *klass);
typedef ir_node *(*construct_instanceof_t)       (ir_node *objptr, ir_type *klass, ir_graph *irg, ir_node *block, ir_node **mem);

void rtti_init(void);
void rtti_construct_runtime_typeinfo(ir_type *klass);
void rtti_lower_InstanceOf(ir_node *instanceof);
void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func);
void rtti_set_instanceof_constructor(construct_instanceof_t func);

ir_entity *rtti_emit_string_const(const char *bytes);

#endif
