
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

typedef struct {
	void *ip;
	void *handler;
} lsda_entry_t;

typedef struct {
	uint32_t n_entries;
	lsda_entry_t entries[1];
} lsda_t;

inline static char *get_string_const_chars(const string_const_t *s)
{
	return (char*)&s->data;
}

inline static uint16_t string_hash(const char* s)
{
	unsigned hash = 0;
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		hash = (31 * hash) + s[i]; // FIXME: this doesn't work for codepoints that are not ASCII.
	}
	return hash & 0xFFFF;
}

inline static bool string_const_equals(const string_const_t *s1, const string_const_t *s2)
{
	if (s1 == s2)                        // referencing the same string_const
		return true;

	if (!s1 || !s2)                      // exactly one of the args is NULL
		return false;

	if (s1->hash != s2->hash)            // cannot be equal if hashes don't match
		return false;

	char *p1 = get_string_const_chars(s1);
	char *p2 = get_string_const_chars(s2);
	while (*p1 != '\0' && *p2 != '\0' && *p1 == *p2) {
		p1++; p2++;
	}
	return (*p1 == '\0' && *p2 == '\0'); // true if both strings reached the end (and thus must be equal)

}

inline static class_info_t *get_class_info(void *obj)
{
	return (class_info_t*)*(unsigned*)((*(unsigned*)obj)-4);
}
#endif
