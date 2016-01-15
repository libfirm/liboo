#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct rtti_t {
	void *dummy;
};
typedef struct rtti_t rtti_t;

struct itt_entry_t {
	void    **itable;
	void     *id;
	int32_t   prev;
	int32_t   next;
};
typedef struct itt_entry_t itt_entry_t;

struct vtable_t {
	rtti_t      *rtti;
	itt_entry_t *itt;
	void        *dynamic_methods[];
};
typedef struct vtable_t vtable_t;

struct object_t {
	vtable_t *vptr;
};
typedef struct object_t object_t;

#endif
