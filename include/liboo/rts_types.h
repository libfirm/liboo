
#ifndef OO_RTS_TYPES_H
#define OO_RTS_TYPES_H

#include <stdint.h>

typedef struct {
	uint16_t hash;
	uint16_t length;
	char     data[1];
} string_const_t;

typedef struct {
	string_const_t *name; /* including signature */
	void         *funcptr;
} method_info_t;

struct _class_info_t;
typedef struct _class_info_t class_info_t;
struct _class_info_t {
	string_const_t *name;
	class_info_t   *superclass;
	int             n_methods;
	method_info_t **methods;

	/* will need a list of interface in order to perform the instance of operation */
};

#endif
