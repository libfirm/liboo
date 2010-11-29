#ifndef RTTI_H
#define RTTI_H

#include <libfirm/firm.h>

typedef void (*construct_runtime_typeinfo_t) (ir_type* klass);

void rtti_init(void);
void rtti_construct_runtime_typeinfo(ir_type *klass);
void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func);

#endif
