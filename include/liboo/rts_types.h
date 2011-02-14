
#ifndef OO_RTS_TYPES_H
#define OO_RTS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	uint16_t hash;
	uint16_t length;
	char     data[1];
} string_const_t;

typedef struct {
	string_const_t *name; /* including signature */
	void           *funcptr;
} method_info_t;

struct _class_info_t;
typedef struct _class_info_t class_info_t;
struct _class_info_t {
	string_const_t *name;
	class_info_t   *superclass;
	int             n_methods;
	method_info_t  *methods;
	int             n_interfaces;
	class_info_t  **interfaces;
};

inline static uint16_t string_hash(const char* s)
{
	unsigned hash = 0;
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		hash = (31 * hash) + s[i]; // FIXME: this doesn't work for codepoints that are not ASCII.
	}
	return hash & 0xFFFF;
}

inline static bool string_const_equals(string_const_t *s1, string_const_t *s2)
{
	if (s1 == s2)                        // referencing the same string_const
		return true;

	if (!s1 || !s2)                      // exactly one of the args is NULL
		return false;

	if (s1->hash != s2->hash)            // cannot be equal if hashes don't match
		return false;

	char *p1 = (char*)&s1->data;
	char *p2 = (char*)&s2->data;
	while (*p1 != '\0' && *p2 != '\0' && *p1 == *p2) {
		p1++; p2++;
	}
	return (*p1 == '\0' && *p2 == '\0'); // true if both strings reached the end (and thus must be equal)

}

#endif
