#include "config.h"

#include "liboo/mangle.h"

#include <string.h>
#include <assert.h>
#include "liboo/oo.h"
#include "adt/cpset.h"
#include "adt/error.h"

#ifdef __APPLE__
static const char *mangle_prefix = "_";
#else
static const char *mangle_prefix = "";
#endif

static struct obstack obst;

static const char* base36 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char *duplicate_string_n(const char* s, size_t n)
{
	char *new_string = XMALLOCN(char, n+1);
	memcpy(new_string, s, n);
	new_string[n] = '\0';
	return new_string;
}

static char *duplicate_string(const char *s)
{
	size_t len = strlen(s);
	return duplicate_string_n(s, len);
}

// (entity name) substitution table
typedef struct {
	char *name;
	char *mangled;
} st_entry;

static cpset_t st;

static int string_cmp (const void *p1, const void *p2)
{
	st_entry *entry1 = (st_entry*) p1;
	st_entry *entry2 = (st_entry*) p2;
	return strcmp(entry1->name, entry2->name) == 0;
}

static unsigned string_hash (const void *obj)
{
	unsigned hash = 0;
	const char *s = ((st_entry*)obj)->name;
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		hash = (31 * hash) + s[i];
	}
	return hash;
}

static void free_ste(st_entry *ste)
{
	if (ste == NULL)
		return;

	free(ste->name);
	free(ste->mangled);
	free(ste);
}

// compression table utility functions
void mangle_ct_init(compression_table_t *ct)
{
	memset(ct, 0, sizeof(compression_table_t));
}

void mangle_ct_flush(compression_table_t *ct)
{
	for (int i = 0; i < COMPRESSION_TABLE_SIZE; i++) {
		if (ct->entries[i].prefix) {
			free(ct->entries[i].prefix);
		}
		ct->entries[i].prefix = NULL;
	}
	ct->next_slot = 0;
}

int mangle_ct_find(compression_table_t* ct, const char* prefix)
{
	for (int i = 0; i < ct->next_slot; i++) {
		if (strcmp(ct->entries[i].prefix, prefix) == 0) {
			return i;
		}
	}
	return -1;
}

void mangle_ct_insert(compression_table_t *ct, const char* prefix)
{
	assert(ct->next_slot < COMPRESSION_TABLE_SIZE);
	ct->entries[ct->next_slot].prefix = duplicate_string(prefix);
	ct->next_slot++;
}

void mangle_emit_substitution(int match, struct obstack *obst)
{
	obstack_1grow(obst, 'S');
	if (match > 0)
		obstack_1grow(obst, base36[match-1]);
	obstack_1grow(obst, '_');
}

void mangle_pointer_type(ir_type *type, struct obstack *obst, compression_table_t *ct) {
	assert (is_Pointer_type(type));
	ir_type *ptarget = get_pointer_points_to_type(type);

	struct obstack uobst; // obstack for unsubstituted type name
	obstack_init(&uobst);

	obstack_1grow(&uobst, '*');
	mangle_type_for_compression_table(ptarget, &uobst);
	const char *unsubstituted = obstack_finish(&uobst);

	// 1. Check if the ct contains the "* ptarget" entry...
	int full_match_p = mangle_ct_find(ct, unsubstituted);
	if (full_match_p > 0) {
		// ...it is available. Emit it and go on.
		mangle_emit_substitution(full_match_p, obst);
		return;
	}

	// 2. Check if the ct contains the "ptarget" (e.g. full match for the target type)...
	int full_match   = mangle_ct_find(ct, unsubstituted+1);
	if (full_match > 0) {
		// ...it does. Emit P and the substitution, then insert the "* ptarget" entry.
		obstack_1grow(obst, 'P');
		mangle_emit_substitution(full_match, obst);
		mangle_ct_insert(ct, unsubstituted);
		return;
	}

	// 3. "ptarget" wasn't known yet. We can emit the P, and recurse...
	obstack_1grow(obst, 'P');
	mangle_type(ptarget, obst, ct);

	// ...mangle_type will introduce the "ptarget" entry if appropriate.
	// This is the right time to insert the "* ptarget" entry.
	mangle_ct_insert(ct, unsubstituted);

	obstack_free(&uobst, NULL);
}

void mangle_primitive_type(ir_type *type, struct obstack *obst)
{
	assert(is_Primitive_type(type));
	const char *tag = get_type_link(type);
	assert(tag != NULL); /* mangle_set_primitive_type_name should have
	                        been called */
	size_t      len = strlen(tag);
	obstack_grow(obst, tag, len);
}

bool mangle_qualified_class_name(ir_type *classtype, struct obstack *obst, compression_table_t *ct)
{
	assert (is_Class_type(classtype));
	if (classtype == get_glob_type())
		return 0;

	ident      *class_ident = get_class_ident(classtype);
	const char *string      = get_id_str(class_ident);
	const char *p           = string;
	size_t      slen        = strlen(string);
	bool        emitted_N   = false;

	// check for a full match, which is not considered a composite name.
	int         full_match  = mangle_ct_find(ct, string);
	if (full_match > 0) {
		mangle_emit_substitution(full_match, obst);
		return emitted_N;
	}

	// no full match, we have to construct a new composite name
	obstack_1grow(obst, 'N');
	emitted_N = true;

	char *request_buffer = XMALLOCN(char, slen+1);

	// search for the longest prefix (component-wise) that is in the ct and use it.
	// New components are emitted as "<length>component" (e.g. "4java2io")
	int   last_match     = -1;
	while (*p != '\0') {
		while (*p == '.')
			++p;
		/* search for '/' or '\0' */
		size_t l;
		for (l = 0; p[l] != '\0' && p[l] != '.'; ++l) {
		}

		const char *comp_begin   = p;
		const char *comp_end     = p + l;
		unsigned    comp_end_idx = (comp_end-string);
		strncpy(request_buffer, string, comp_end_idx);
		request_buffer[comp_end_idx] = '\0';
		p = comp_end;

		int match = mangle_ct_find(ct, request_buffer);
		if (match >= 0) {
			last_match = match;
		} else {
			mangle_ct_insert(ct, request_buffer);

			if (last_match >= 0) {
				mangle_emit_substitution(last_match, obst);
				last_match = -1;
			}
			obstack_printf(obst, "%d", l);
			obstack_grow(obst, comp_begin, l);
		}
	}

	free(request_buffer);

	// FIXME: support type parameter

	return emitted_N;
}

void mangle_type(ir_type *type, struct obstack *obst, compression_table_t *ct)
{
	if (is_Pointer_type(type)) {
		mangle_pointer_type(type, obst, ct);
	} else if (is_Primitive_type(type)) {
		mangle_primitive_type(type, obst);
	} else if (is_Class_type(type)) {
		bool emitted_N = mangle_qualified_class_name(type, obst, ct);
		if (emitted_N)
			obstack_1grow(obst, 'E');
	}
}

void mangle_type_for_compression_table(ir_type *type, struct obstack *obst)
{
	if (is_Primitive_type(type)) {
		mangle_primitive_type(type, obst);
	} else if (is_Class_type(type)) {
		obstack_printf(obst, "%s", get_class_name(type));
		//FIXME: Support type parameter
	} else if (is_Pointer_type(type)) {
		obstack_1grow(obst, '*');
		mangle_type_for_compression_table(get_pointer_points_to_type(type), obst);
	} else {
		assert (0);
	}
}

/**
 *  mangles in a C++ like fashion so we can use c++filt to demangle
 */
ident *mangle_entity_name(ir_entity *entity)
{
	assert(obstack_object_size(&obst) == 0);
	assert(entity != NULL);

	compression_table_t ct;
	mangle_ct_init(&ct);

	ir_type *owner  = get_entity_owner(entity);
	ir_type *alt_ns = oo_get_entity_alt_namespace(entity);
	ir_type *type   = get_entity_type(entity);
	ir_type *glob   = get_glob_type();

	ir_type *ns     = alt_ns != NULL ? alt_ns : owner;

	if (ns == glob)
		return id_mangle(new_id_from_str(mangle_prefix), get_entity_ident(entity));

	obstack_grow(&obst, mangle_prefix, strlen(mangle_prefix));
	obstack_grow(&obst, "_Z", 2);

	mangle_qualified_class_name(ns, &obst, &ct);

	/* mangle entity name */
	const char *name_sig  = get_entity_name(entity);
	const char *p         = name_sig;

	// strip signature from the entity name (XXX: this might be Java specific)
	while (*p != '\0' && *p != '.') p++;
	size_t      len       = (size_t)(p-name_sig);
	char       *name_only = duplicate_string_n(name_sig, len);

	st_entry ste;
	ste.name = name_only;
	st_entry *found_ste = cpset_find(&st, &ste);
	if (found_ste == NULL) {
		obstack_printf(&obst, "%d%s", (int) len, name_only);
	} else {
		obstack_printf(&obst, "%s", found_ste->mangled);
	}

	obstack_1grow(&obst, 'E');

	if (! is_Method_type(type))
		goto name_finished;

	if (! oo_get_method_is_constructor(entity)) {
		obstack_1grow(&obst, 'J');

		/* mangle return type */
		int n_ress = get_method_n_ress(type);
		if (n_ress == 0) {
			obstack_1grow(&obst, 'v');
		} else {
			assert(n_ress == 1);
			mangle_type(get_method_res_type(type, 0), &obst, &ct);
		}
	}

	/* mangle parameter types */
	int n_params = get_method_n_params(type);
	int start    = (owner == glob) ? 0 : 1; // skip implicit this param of non-static methods
	if (n_params-start == 0) {
		obstack_1grow(&obst, 'v');
	} else {
		for (int i = start; i < n_params; ++i) {
			ir_type *parameter = get_method_param_type(type, i);
			mangle_type(parameter, &obst, &ct);
		}
	}

name_finished: ;
	size_t  result_len    = obstack_object_size(&obst);
	char   *result_string = obstack_finish(&obst);
	ident  *result        = new_id_from_chars(result_string, result_len);
	obstack_free(&obst, result_string);

	free(name_only);

	mangle_ct_flush(&ct);
	return result;
}

ident *mangle_vtable_name(ir_type *clazz)
{
	assert(obstack_object_size(&obst) == 0);
	assert(clazz != NULL && is_Class_type(clazz));

	compression_table_t ct;
	mangle_ct_init(&ct);

	obstack_grow(&obst, mangle_prefix, strlen(mangle_prefix));
	obstack_grow(&obst, "_ZTV", 4);

	int emitted_N = mangle_qualified_class_name(clazz, &obst, &ct);
	assert(emitted_N);

	obstack_1grow(&obst, 'E');

	size_t  result_len    = obstack_object_size(&obst);
	char   *result_string = obstack_finish(&obst);
	ident  *result        = new_id_from_chars(result_string, result_len);
	obstack_free(&obst, result_string);

	mangle_ct_flush(&ct);
	return result;
}

static void clear_primitive_type_link(type_or_ent tore, void *env)
{
	(void) env;
	if (get_kind(tore.ent) == k_type) {
		ir_type *type = tore.typ;
		if (is_Primitive_type(type)) {
			set_type_link(type, NULL);
		}
	}
}

void mangle_init(void)
{
	obstack_init(&obst);
	cpset_init(&st, string_hash, string_cmp);

	/* just to be sure, clear all type links */
	type_walk(clear_primitive_type_link, NULL, NULL);
}

void mangle_set_primitive_type_name(ir_type *type, const char *name)
{
	assert(is_Primitive_type(type));
	char *duplicate_name = obstack_copy(&obst, name, strlen(name)+1);
	set_type_link(type, duplicate_name);
}

void mangle_add_name_substitution(const char *name, const char *mangled)
{
	st_entry *ste = XMALLOC(st_entry);
	ste->name = duplicate_string(name);
	ste->mangled = duplicate_string(mangled);
	st_entry* obj = (st_entry*) cpset_insert(&st, ste);
	/* noone should insert 2 substitutions for the same name */
	if (obj != ste)
		panic("more than 1 substitution for name '%s'\n", name);
}

void mangle_deinit(void)
{
	obstack_free(&obst, NULL);

	cpset_iterator_t iter;
	cpset_iterator_init(&iter, &st);

	st_entry *cur_ste;
	while ( (cur_ste = (st_entry*)cpset_iterator_next(&iter)) != NULL) {
		free_ste(cur_ste);
	}

	cpset_destroy(&st);
}
