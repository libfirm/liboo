/**
 * @file
 * Structure of runtime type information as emitted by liboo.
 *
 * Warning: if you change this struct, also change the code in rtti.c which
 * emits this data during a compilation run
 */
#ifndef OO_RTS_TYPES_H
#define OO_RTS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	uint32_t hash;
	char     data[];
} string_const_t;

typedef struct {
	string_const_t *name; /* including signature */
	void           *funcptr;
} method_info_t;

struct class_info_t {
	string_const_t       *name;
	uint32_t              uid;
	size_t                size;
	struct class_info_t  *superclass;
	uint32_t              n_methods;
	method_info_t        *methods;
	uint32_t              n_interfaces;
	struct class_info_t **interfaces;
};
typedef struct class_info_t class_info_t;

inline static const char *get_string_const_chars(const string_const_t *s)
{
	return s->data;
}

inline static uint32_t string_hash(const char* s)
{
	uint32_t hash = 0;
	for (const char *c = s; *c != '\0'; ++c) {
		hash = (31 * hash) ^ *c;
	}
	return hash;
}

inline static bool string_const_equals(const string_const_t *s1,
                                       const string_const_t *s2)
{
	/* cannot be equal if hashes don't match */
	if (s1->hash != s2->hash)
		return false;
	if (s1 == s2)
		return true;

	char *p1 = (char*)&s1->data;
	char *p2 = (char*)&s2->data;
	while (*p1 != '\0' && *p2 != '\0' && *p1 == *p2) {
		p1++; p2++;
	}
	/* true if both strings reached the end (and thus must be equal) */
	return (*p1 == '\0' && *p2 == '\0');

}

#endif
