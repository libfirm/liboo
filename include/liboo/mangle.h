#ifndef OO_MANGLE_H
#define OO_MANGLE_H

#include <stdbool.h>
#include <libfirm/firm.h>
#include "../src-cpp/adt/obst.h"

/**
 * Mangle function type into function names to make polymorphic (overloaded)
 * functions unique
 */

/*
 * FIXME: de-Java-rize example
 *
 * The compression table contains prefixes of currently mangled name.
 * Pointers to and arrays of a specific type, and the JArray keyword, cause an additional entry.
 * Example: mangling
 * JArray<java::lang::Object*>* java::lang::ClassLoader::putDeclaredAnnotations(java::lang::Class*, int, int, int, JArray<java::lang::Object*>*)
 * S_  = java
 * S0_ = java/lang
 * S1_ = java/lang/ClassLoader
 * S2_ = JArray
 * S3_ = java/lang/Object
 * S4_ = Pjava/lang/Object
 * S5_ = JArray<Pjava/lang/Object>
 * S6_ = PJArray<Pjava/lang/Object>
 * => _ZN4java4lang11ClassLoader22putDeclaredAnnotationsEJP6JArrayIPNS0_6ObjectEEPNS0_5ClassEiiiS6_
 */

#define COMPRESSION_TABLE_SIZE 36 // theoretically, there could be substitution patterns with more than one digit.

typedef struct {
	char *prefix;
} compression_table_entry_t;

typedef struct {
	compression_table_entry_t entries[COMPRESSION_TABLE_SIZE];
	int                       next_slot;
} compression_table_t;

void mangle_ct_init(compression_table_t *ct);
void mangle_ct_flush(compression_table_t *ct);
int  mangle_ct_find(compression_table_t* ct, const char* prefix);
void mangle_ct_insert(compression_table_t *ct, const char* prefix);
void mangle_emit_substitution(int match, struct obstack *obst);

void mangle_pointer_type(ir_type *type, struct obstack *obst, compression_table_t *ct);
void mangle_primitive_type(ir_type *type, struct obstack *obst);
bool mangle_qualified_class_name(ir_type *type, struct obstack *obst, compression_table_t *ct);
void mangle_type(ir_type *type, struct obstack *obst, compression_table_t *ct);
void mangle_type_for_compression_table(ir_type *type, struct obstack *obst);

ident *mangle_entity_name(ir_entity *entity);
ident *mangle_vtable_name(ir_type *clazz);

void mangle_init(void);
void mangle_set_primitive_type_name(ir_type *type, const char *name);
void mangle_add_name_substitution(const char *name, const char *mangled);
void mangle_deinit(void);

#endif
