#include "config.h"

#include "liboo/rtti.h"
#include "liboo/oo.h"
#include "liboo/rts_types.h"
#include "liboo/nodes.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "adt/error.h"
#include "adt/cpset.h"
#include "adt/obst.h"
#include "adt/util.h"

static ir_type   *class_info;
static ir_entity *class_info_name;
static ir_entity *class_info_uid;
static ir_entity *class_info_size;
static ir_entity *class_info_superclass;
static ir_entity *class_info_n_methods;
static ir_entity *class_info_methods;
static ir_entity *class_info_n_interfaces;
static ir_entity *class_info_interfaces;

static ir_type   *method_info;
static ir_entity *method_info_name;
static ir_entity *method_info_funcptr;

static ir_type   *method_info_array;
static ir_type   *reference_array;

static ir_type   *string_const;
static ir_entity *string_const_hash;
static ir_entity *string_const_data;

static ir_entity *default_instanceof_entity;

static cpset_t string_constant_pool;

typedef struct {
	char      *string;
	ir_entity *entity;
} scp_entry_t;

static construct_runtime_typeinfo_t construct_runtime_typeinfo;
static construct_instanceof_t       construct_instanceof;

static void free_scpe(scp_entry_t *scpe)
{
	if (scpe == NULL)
		return;

	free(scpe->string);
	free(scpe);
}

static int scp_cmp_function(const void *p1, const void *p2)
{
	scp_entry_t *scpe1 = (scp_entry_t*) p1;
	scp_entry_t *scpe2 = (scp_entry_t*) p2;

	return strcmp(scpe1->string, scpe2->string) == 0;
}

static unsigned scp_hash_function(const void *obj)
{
	scp_entry_t *scpe = (scp_entry_t*) obj;
	return string_hash(scpe->string);
}

static ir_initializer_t *new_initializer_reference(ir_entity *entity)
{
	ir_graph *irg      = get_const_code_irg();
	ir_node  *symconst = new_r_Address(irg, entity);
	return create_initializer_const(symconst);
}

static ir_initializer_t *new_initializer_long(long val, const ir_type *type)
{
	ir_mode   *mode = get_type_mode(type);
	ir_tarval *tv   = new_tarval_from_long(val, mode);
	assert(tv != tarval_bad);
	return create_initializer_tarval(tv);
}

/**
 * Initialize types used for runtime type information. We hope that this is
 * the same that the C-compiler compiling the c-part of the runtime library
 * produces (otherwise we have to think of something more robust).
 */
static void init_rtti_firm_types(void)
{
	ir_type *type_reference = get_type_for_mode(mode_P);
	ir_type *type_uint32_t  = get_type_for_mode(mode_Iu);
	ir_mode *mode_size_t    = get_reference_mode_unsigned_eq(mode_P);
	ir_type *type_size_t    = get_type_for_mode(mode_size_t);
	ir_type *type_char      = get_type_for_mode(mode_Bu);
	ir_type *type_int       = get_type_for_mode(mode_Is);

	ident *id = new_id_from_str("class_info$");
	class_info = new_type_struct(id);

	id = new_id_from_str("name");
	class_info_name = new_entity(class_info, id, type_reference);
	id = new_id_from_str("uid");
	class_info_uid = new_entity(class_info, id, type_uint32_t);
	id = new_id_from_str("size");
	class_info_size = new_entity(class_info, id, type_size_t);
	id = new_id_from_str("superclass");
	class_info_superclass = new_entity(class_info, id, type_reference);
	id = new_id_from_str("n_methods");
	class_info_n_methods = new_entity(class_info, id, type_uint32_t);
	id = new_id_from_str("methods");
	class_info_methods = new_entity(class_info, id, type_reference);
	id = new_id_from_str("n_interfaces");
	class_info_n_interfaces = new_entity(class_info, id, type_uint32_t);
	id = new_id_from_str("interfaces");
	class_info_interfaces = new_entity(class_info, id, type_reference);
	default_layout_compound_type(class_info);
	/* I'd really like to use the following assert, unfortunately it is
	 * useless when we are cross-compiling. And I see no easy way at the
	 * moment to check wether we are.
	 * assert(get_type_size_bytes(class_info) == sizeof(class_info_t));
	 */

	id = new_id_from_str("method_info$");
	method_info = new_type_struct(id);
	id = new_id_from_str("method_info_name");
	method_info_name = new_entity(method_info, id, type_reference);
	id = new_id_from_str("method_info_funcptr");
	method_info_funcptr = new_entity(method_info, id, type_reference);
	default_layout_compound_type(method_info);
	/* assert(get_type_size_bytes(method_info) == sizeof(method_info_t)); */

	method_info_array = new_type_array(method_info);
	set_array_variable_size(method_info_array, 1);

	id = new_id_from_str("string_const$");
	string_const = new_type_struct(id);
	id = new_id_from_str("hash");
	string_const_hash = new_entity(string_const, id, type_uint32_t);
	ir_type *type_char_array = new_type_array(type_char);
	set_array_variable_size(type_char_array, 1);
	id = new_id_from_str("data");
	string_const_data = new_entity(string_const, id, type_char_array);
	set_compound_variable_size(string_const, 1);
	default_layout_compound_type(string_const);
	/* assert(get_type_size_bytes(string_const) == sizeof(string_const_t)); */

	reference_array = new_type_array(type_reference);
	set_array_variable_size(reference_array, 1);

	ir_type *default_io_type = new_type_method(2, 1);
	set_method_param_type(default_io_type, 0, type_reference);
	set_method_param_type(default_io_type, 1, type_reference);
	set_method_res_type(default_io_type, 0, type_int);
	ident *default_io_ident = new_id_from_str("oo_rt_instanceof");
	default_instanceof_entity = create_compilerlib_entity(default_io_ident, default_io_type);
}

ir_entity *rtti_emit_string_const(const char *string)
{
	scp_entry_t lookup_scpe;
	lookup_scpe.string = (char*) string;

	scp_entry_t *found_scpe = cpset_find(&string_constant_pool, &lookup_scpe);
	if (found_scpe != NULL) {
		ir_entity *string_const = found_scpe->entity;
		assert(is_entity(string_const));
		return string_const;
	}

	size_t            n_members   = get_compound_n_members(string_const);
	ir_initializer_t *initializer = create_initializer_compound(n_members);
	size_t            i           = 0;

	uint32_t          hash      = string_hash(string);
	ir_type          *hash_type = get_entity_type(string_const_hash);
	ir_initializer_t *hash_init = new_initializer_long(hash, hash_type);
	set_initializer_compound_value(initializer, i++, hash_init);

	size_t   len       = strlen(string)+1; /* incl. the '\0' end marker */
	ir_type *type_data = get_entity_type(string_const_data);
	ir_type *type_char = get_array_element_type(type_data);
	ir_initializer_t *data_init = create_initializer_compound(len);
	for (size_t i = 0; i < len; ++i) {
		ir_initializer_t *char_init
			= new_initializer_long(string[i], type_char);
		set_initializer_compound_value(data_init, i, char_init);
	}
	set_initializer_compound_value(initializer, i++, data_init);
	assert(i == n_members);

	ident     *id     = id_unique("rtti_string_%u");
	ir_type   *glob   = get_glob_type();
	ir_entity *entity = new_entity(glob, id, string_const);
	set_entity_visibility(entity, ir_visibility_private);
	set_entity_linkage(entity, IR_LINKAGE_CONSTANT);
	set_entity_initializer(entity, initializer);

	scp_entry_t *new_scpe = XMALLOC(scp_entry_t);
	new_scpe->string      = XMALLOCN(char, len);
	memcpy(new_scpe->string, string, len);

	new_scpe->entity = entity;
	cpset_insert(&string_constant_pool, new_scpe);

	return entity;
}

static ir_initializer_t *create_method_info(ir_entity *method)
{
	size_t            n_members   = get_compound_n_members(method_info);
	ir_initializer_t *initializer = create_initializer_compound(n_members);
	size_t            i           = 0;

	const char *method_name_str = get_entity_name(method);
	ir_entity  *method_name_ent = rtti_emit_string_const(method_name_str);
	ir_initializer_t *method_name_init
		= new_initializer_reference(method_name_ent);
	set_initializer_compound_value(initializer, i++, method_name_init);

	ir_initializer_t *method_init;
	if (oo_get_method_is_abstract(method)) {
		method_init = get_initializer_null();
	} else {
		method_init = new_initializer_reference(method);
	}
	set_initializer_compound_value(initializer, i++, method_init);
	assert(i == n_members);

	return initializer;
}

static ir_entity *create_method_table(ir_type *klass, size_t n_methods)
{
	ir_initializer_t *initializer = create_initializer_compound(n_methods);
	size_t            i           = 0;

	size_t n_members = get_compound_n_members(klass);
	for (size_t m = 0; m < n_members; ++m) {
		ir_entity *member = get_class_member(klass, m);
		if (!is_method_entity(member))
			continue;
		if (oo_get_method_exclude_from_vtable(member))
			continue;

		ir_entity *bound = ddispatch_get_bound_entity(member);
		ir_initializer_t *mt_init = create_method_info(bound);
		set_initializer_compound_value(initializer, i++, mt_init);
	}
	assert(i == n_methods);

	ident     *id     = id_unique("rtti_mt_%u");
	ir_type   *glob   = get_glob_type();
	ir_entity *entity = new_entity(glob, id, method_info_array);
	set_entity_visibility(entity, ir_visibility_private);
	set_entity_linkage(entity, IR_LINKAGE_CONSTANT);
	set_entity_initializer(entity, initializer);

	return entity;
}

static ir_entity *create_interface_table(ir_type *klass, size_t n_interfaces)
{
	ir_initializer_t *initializer = create_initializer_compound(n_interfaces);
	size_t            i           = 0;

	size_t n_supertypes = get_class_n_supertypes(klass);
	for (size_t s = 0; s < n_supertypes; ++s) {
		ir_type *iface = get_class_supertype(klass, s);
		if (!oo_get_class_is_interface(iface))
			continue;

		ir_entity        *iface_rtti = oo_get_class_rtti_entity(iface);
		ir_initializer_t *iface_init = new_initializer_reference(iface_rtti);
		set_initializer_compound_value(initializer, i++, iface_init);
	}
	assert(i == n_interfaces);

	ident     *id     = id_unique("rtti_it_%u");
	ir_type   *glob   = get_glob_type();
	ir_entity *entity = new_entity(glob, id, reference_array);
	set_entity_visibility(entity, ir_visibility_private);
	set_entity_linkage(entity, IR_LINKAGE_CONSTANT);
	set_entity_initializer(entity, initializer);

	return entity;
}

void rtti_default_construct_runtime_typeinfo(ir_type *klass)
{
	assert(is_Class_type(klass));
	ir_entity *rtti_entity = oo_get_class_rtti_entity(klass);
	if (rtti_entity == NULL)
		return;

	size_t            n_init      = get_compound_n_members(class_info);
	ir_initializer_t *initializer = create_initializer_compound(n_init);
	size_t            i           = 0;

	ident            *tname_id   = get_class_ident(klass);
	ir_entity        *tname_ent  = rtti_emit_string_const(get_id_str(tname_id));
	ir_initializer_t *tname_init = new_initializer_reference(tname_ent);
	set_initializer_compound_value(initializer, i++, tname_init);

	uint32_t          uid        = oo_get_class_uid(klass);
	ir_type          *uid_type   = get_entity_type(class_info_uid);
	ir_initializer_t *uid_init   = new_initializer_long(uid, uid_type);
	set_initializer_compound_value(initializer, i++, uid_init);

	ir_graph         *const_code_irg = get_const_code_irg();
	ir_type          *size_type      = get_entity_type(class_info_size);
	ir_mode          *size_mode      = get_type_mode(size_type);
	ir_node          *size_node      = new_r_Size(const_code_irg, size_mode, klass);
	ir_initializer_t *size_init      = create_initializer_const(size_node);
	set_initializer_compound_value(initializer, i++, size_init);

	ir_entity *superclass_rtti = NULL;
	size_t n_supertypes = get_class_n_supertypes(klass);
	for (size_t i = 0; i < n_supertypes; ++i) {
		ir_type *supertype = get_class_supertype(klass, i);
		if (oo_get_class_is_interface(supertype))
			continue;
		assert(superclass_rtti == NULL);
		superclass_rtti = oo_get_class_rtti_entity(supertype);
		assert(superclass_rtti != NULL);
	}
	ir_initializer_t *superclass_init;
	if (superclass_rtti != NULL) {
		superclass_init = new_initializer_reference(superclass_rtti);
	} else {
		superclass_init = get_initializer_null();
	}
	set_initializer_compound_value(initializer, i++, superclass_init);

	size_t n_methods = 0;
	size_t n_members = get_class_n_members(klass);
	for (size_t i = 0; i < n_members; i++) {
		ir_entity *member = get_class_member(klass, i);
		if (!is_method_entity(member))
			continue;
		if (oo_get_method_exclude_from_vtable(member))
			continue;
		++n_methods;
	}
	ir_type *n_methods_type = get_entity_type(class_info_n_methods);
	ir_initializer_t *n_methods_init
		= new_initializer_long(n_methods, n_methods_type);
	set_initializer_compound_value(initializer, i++, n_methods_init);

	ir_initializer_t *methods_init;
	if (n_methods > 0) {
		ir_entity *method_table = create_method_table(klass, n_methods);
		methods_init = new_initializer_reference(method_table);
	} else {
		methods_init = get_initializer_null();
	}
	set_initializer_compound_value(initializer, i++, methods_init);

	size_t n_interfaces = 0;
	for (size_t i = 0; i < n_supertypes; i++) {
		ir_type *supertype = get_class_supertype(klass, i);
		if (!oo_get_class_is_interface(supertype))
			continue;
		++n_interfaces;
	}
	ir_type *n_interfaces_type = get_entity_type(class_info_n_interfaces);
	ir_initializer_t *n_interfaces_init
		= new_initializer_long(n_interfaces, n_interfaces_type);
	set_initializer_compound_value(initializer, i++, n_interfaces_init);

	ir_initializer_t *interfaces_init;
	if (n_interfaces > 0) {
		ir_entity *iface_table = create_interface_table(klass, n_interfaces);
		interfaces_init = new_initializer_reference(iface_table);
	} else {
		interfaces_init = get_initializer_null();
	}
	set_initializer_compound_value(initializer, i++, interfaces_init);

	assert(i == n_init);

	set_entity_type(rtti_entity, class_info);
	set_entity_initializer(rtti_entity, initializer);
	add_entity_linkage(rtti_entity, IR_LINKAGE_CONSTANT);
}

ir_node *rtti_default_construct_instanceof(ir_node *objptr, ir_type *klass, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_type    *type_reference = get_type_for_mode(mode_P);
	ir_node    *cur_mem      = *mem;

	// we need the reference to the object's class$ field
	// first, dereference the vptr in order to get the vtable address.
	ir_entity  *vptr_entity  = oo_get_class_vptr_entity(klass); // XXX: this is a bit weird and works iff the vptr entity is the same in the whole type hierarchy.
	ir_node    *vptr_addr    = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, vptr_entity);
	ir_node    *vptr_load    = new_r_Load(block, cur_mem, vptr_addr, mode_P, cons_none);
	ir_node    *vtable_addr  = new_r_Proj(vptr_load, mode_P, pn_Load_res);
	            cur_mem      = new_r_Proj(vptr_load, mode_M, pn_Load_M);

	// second, calculate the position of the RTTI ref in relation to the target of vptr and dereference it.
	int         offset       = (ddispatch_get_index_of_rtti_ptr() - ddispatch_get_vptr_points_to_index()) * get_type_size_bytes(type_reference);
	ir_node    *obj_ci_offset= new_r_Const_long(irg, mode_P, offset);
	ir_node    *obj_ci_add   = new_r_Add(block, vtable_addr, obj_ci_offset, mode_P);
	ir_node    *obj_ci_load  = new_r_Load(block, cur_mem, obj_ci_add, mode_P, cons_none);
	ir_node    *obj_ci_ref   = new_r_Proj(obj_ci_load, mode_P, pn_Load_res);
	            cur_mem      = new_r_Proj(obj_ci_load, mode_M, pn_Load_M);

	// get a symconst to klass' classinfo.
	ir_entity  *test_ci      = oo_get_class_rtti_entity(klass);
	assert(test_ci);
	ir_node    *test_ci_ref  = new_r_Address(irg, test_ci);

	ir_node   *callee        = new_r_Address(irg, default_instanceof_entity);
	ir_node   *args[2]       = { obj_ci_ref, test_ci_ref };
	ir_type   *call_type     = get_entity_type(default_instanceof_entity);
	ir_node   *call          = new_r_Call(block, cur_mem, callee, 2, args, call_type);
	           cur_mem       = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node   *ress          = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node   *res           = new_r_Proj(ress, mode_Is, 0);
	ir_node   *res_b         = new_r_Cmp(block, res, new_r_Const_long(irg, mode_Is, 0), ir_relation_less_greater);

	*mem = cur_mem;

	return res_b;
}

void rtti_init()
{
	construct_runtime_typeinfo = rtti_default_construct_runtime_typeinfo;
	construct_instanceof = rtti_default_construct_instanceof;

	init_rtti_firm_types();
	cpset_init(&string_constant_pool, scp_hash_function, scp_cmp_function);
}

void rtti_deinit()
{
	cpset_iterator_t iter;
	cpset_iterator_init(&iter, &string_constant_pool);

	scp_entry_t *cur_scpe;
	while ((cur_scpe = (scp_entry_t*)cpset_iterator_next(&iter)) != NULL) {
		free_scpe(cur_scpe);
	}

	cpset_destroy(&string_constant_pool);
}

void rtti_construct_runtime_typeinfo(ir_type *klass)
{
	construct_runtime_typeinfo(klass);
}

void rtti_lower_InstanceOf(ir_node *instanceof)
{
	ir_node  *objptr  = get_InstanceOf_ptr(instanceof);
	ir_type  *type    = get_InstanceOf_type(instanceof);
	ir_node  *block   = get_nodes_block(instanceof);
	ir_graph *irg     = get_irn_irg(instanceof);
	ir_node  *cur_mem = get_InstanceOf_mem(instanceof);
	ir_node  *res     = construct_instanceof(objptr, type, irg, block, &cur_mem);

	ir_node *in[pn_InstanceOf_max+1] = {
		[pn_InstanceOf_M]   = cur_mem,
		[pn_InstanceOf_res] = res,
	};
	turn_into_tuple(instanceof, ARRAY_SIZE(in), in);
}

void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func)
{
	assert(func != NULL);
	construct_runtime_typeinfo = func;
}

void rtti_set_instanceof_constructor(construct_instanceof_t func)
{
	assert(func != NULL);
	construct_instanceof = func;
}
