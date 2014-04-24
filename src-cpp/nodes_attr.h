#ifndef OO_NODES_ATTR_H
#define OO_NODES_ATTR_H

#include <libfirm/firm_types.h>

typedef struct op_InstanceOf_attr_t {
	ir_type *type;
} op_InstanceOf_attr_t;

typedef struct op_MethodSel_attr_t {
	ir_entity *entity;
} op_MethodSel_attr_t;

typedef struct op_VptrIsSet_attr_t {
	ir_type *type;
} op_VptrIsSet_attr_t;

#endif
