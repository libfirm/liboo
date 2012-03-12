#ifndef OO_RTS_TYPES_H
#define OO_RTS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	uint16_t hash;
	uint16_t length;
	char     data[];
} string_const_t;

typedef struct {
	string_const_t *name; /* including signature */
	void           *funcptr;
} method_info_t;

struct class_info_t {
	string_const_t       *name;
	struct class_info_t  *superclass;
	int                   n_methods;
	method_info_t        *methods;
	int                   n_interfaces;
	struct class_info_t **interfaces;
};
typedef struct class_info_t class_info_t;

inline static const char *get_string_const_chars(const string_const_t *s)
{
	return s->data;
}

inline static uint16_t string_hash(const char* s)
{
	unsigned hash = 0;
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		hash = (31 * hash) + s[i];
	}
	return hash & 0xFFFF;
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
