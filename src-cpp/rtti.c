
#include <liboo/rtti.h>

#include <assert.h>
#include "adt/error.h"

static struct {
	construct_runtime_typeinfo_t construct_runtime_typeinfo;
} rtti_model;

static void default_construct_runtime_typeinfo(ir_type *klass)
{
	(void) klass;
	panic("default_construct_runtime_typeinfo NYI.");
}

void rtti_init()
{
	rtti_model.construct_runtime_typeinfo = default_construct_runtime_typeinfo;
}

void rtti_construct_runtime_typeinfo(ir_type *klass)
{
	(*rtti_model.construct_runtime_typeinfo)(klass);
}

void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func)
{
	assert (func);
	rtti_model.construct_runtime_typeinfo = func;
}
