#include "config.h"

#include "liboo/rtti.h"
#include "liboo/oo.h"
#include "liboo/rts_types.h"
#include "liboo/oo_nodes.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "adt/error.h"
#include "adt/cpset.h"
#include "adt/obst.h"

static ir_mode *mode_uint16_t;
static ir_type *type_uint16_t;
static ir_type *type_reference;
static ir_type *type_int;

static ir_entity *default_instanceof_entity;

static cpset_t string_constant_pool;

typedef struct {
	char      *string;
	ir_entity *entity;
} scp_entry_t;

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

static struct {
	construct_runtime_typeinfo_t construct_runtime_typeinfo;
	construct_instanceof_t       construct_instanceof;
} rtti_model;

ir_entity *rtti_emit_string_const(const char *bytes)
{
	scp_entry_t lookup_scpe;
	lookup_scpe.string = (char*) bytes;

	scp_entry_t *found_scpe = cpset_find(&string_constant_pool, &lookup_scpe);
	if (found_scpe != NULL) {
		ir_entity *string_const = found_scpe->entity;
		assert (is_entity(string_const));
		return string_const;
	}

	size_t            len        = strlen(bytes);
	size_t            len0       = len + 1; // incl. the '\0' byte
	uint16_t          hash       = string_hash(bytes);

	ir_graph         *ccode      = get_const_code_irg();

	ident            *id         = id_unique("_String_%u_");
	ir_type          *strc_type  = new_type_struct(id_mangle(id, new_id_from_str("type")));
	ir_initializer_t *cinit      = create_initializer_compound(3);

	// hash
	ir_entity        *hash_ent   = new_entity(strc_type, new_id_from_str("hash"), type_uint16_t);
	ir_initializer_t *hash_init  = create_initializer_const(new_r_Const_long(ccode, mode_uint16_t, hash));
	set_entity_initializer(hash_ent, hash_init);
	set_initializer_compound_value(cinit, 0, hash_init);

	// length
	ir_entity        *length_ent = new_entity(strc_type, new_id_from_str("length"), type_uint16_t);
	ir_initializer_t *length_init= create_initializer_const(new_r_Const_long(ccode, mode_uint16_t, len));
	set_entity_initializer(length_ent, length_init);
	set_initializer_compound_value(cinit, 1, length_init);

	// data
	ir_mode          *el_mode    = mode_Bu;
	ir_type          *el_type    = new_type_primitive(el_mode);
	ir_type          *array_type = new_type_array(1, el_type);

	set_array_lower_bound_int(array_type, 0, 0);
	set_array_upper_bound_int(array_type, 0, len0);
	set_type_size_bytes(array_type, len0);
	set_type_state(array_type, layout_fixed);

	ir_entity        *data_ent   = new_entity(strc_type, new_id_from_str("data"), array_type);

	// initialize each array element to an input byte
	ir_initializer_t *data_init  = create_initializer_compound(len0);
	for (size_t i = 0; i < len0; ++i) {
		ir_tarval        *tv  = new_tarval_from_long(bytes[i], el_mode);
		ir_initializer_t *val = create_initializer_tarval(tv);
		set_initializer_compound_value(data_init, i, val);
	}
	set_entity_initializer(data_ent, data_init);
	set_initializer_compound_value(cinit, 2, data_init);

	set_type_size_bytes(strc_type, get_type_size_bytes(type_uint16_t)*2 + get_type_size_bytes(array_type));
	default_layout_compound_type(strc_type);

	// finally, the entity for the string constant
	ir_entity *strc   = new_entity(get_glob_type(), id, strc_type);
	set_entity_initializer(strc, cinit);
	set_entity_ld_ident(strc, id);

	scp_entry_t *new_scpe = XMALLOC(scp_entry_t);
	new_scpe->string = XMALLOCN(char, len0);
	for (unsigned i = 0; i < len0; i++)
		new_scpe->string[i] = bytes[i];

	new_scpe->entity = strc;
	cpset_insert(&string_constant_pool, new_scpe);

	return strc;
}

static ir_node *create_ccode_symconst(ir_entity *ent)
{
	ir_graph *ccode = get_const_code_irg();
	symconst_symbol sym;
	sym.entity_p = ent;
	ir_node *symc = new_r_SymConst(ccode, mode_P, sym, symconst_addr_ent);
	return symc;
}

static ir_entity *emit_primitive_member(ir_type *owner, const char *name, ir_type *type, ir_node *value)
{
	assert (is_Primitive_type(type));
	ident            *id   = new_id_from_str(name);
	ir_entity        *ent  = new_entity(owner, id, type);
	ir_initializer_t *init = create_initializer_const(value);
	set_entity_initializer(ent, init);
	return ent;
}

#define MD_SIZE_BYTES (get_type_size_bytes(type_reference)*2)
static ir_entity *emit_method_desc(ir_type *owner, ir_entity *ent)
{
	ident            *id            = id_unique("_MD_%u_");
	ir_type          *md_type       = new_type_struct(id_mangle(id, new_id_from_str("type")));

	ir_initializer_t *cinit         = create_initializer_compound(2);

	const char       *name_str      = get_entity_name(ent);
	ir_entity        *name_const_ent= rtti_emit_string_const(name_str);
	ir_entity        *name_ent      = emit_primitive_member(md_type, "name", type_reference, create_ccode_symconst(name_const_ent));
	set_initializer_compound_value(cinit, 0, get_entity_initializer(name_ent));

	ir_node          *funcptr       = NULL;
	if (oo_get_method_is_abstract(ent)) {
		funcptr = new_r_Const_long(get_const_code_irg(), mode_P, 0);
	} else {
		funcptr = create_ccode_symconst(ent);
	}

	ir_entity        *funcptr_ent   = emit_primitive_member(md_type, "funcptr", type_reference, funcptr);
	set_initializer_compound_value(cinit, 1, get_entity_initializer(funcptr_ent));

	set_type_size_bytes(md_type, MD_SIZE_BYTES);
	default_layout_compound_type(md_type);

	ir_entity        *md_ent        = new_entity(owner, id, md_type);
	set_entity_initializer(md_ent, cinit);
	set_entity_ld_ident(md_ent, id);

	return md_ent;
}

static ir_entity *emit_method_table(ir_type *klass, int n_methods)
{
	ident            *id            = id_unique("_MT_%u_");
	ir_type          *mt_type       = new_type_struct(id_mangle(id, new_id_from_str("type")));

	int               n_members     = get_class_n_members(klass);
	ir_initializer_t *cinit         = create_initializer_compound(n_methods);
	int               cur_init_slot = 0;

	for (int i = 0; i < n_members; i++) {
		ir_entity *member = get_class_member(klass, i);
		if (! is_method_entity(member) || oo_get_method_exclude_from_vtable(member)) continue;

		ir_entity *md_ent = emit_method_desc(mt_type, member);
		set_initializer_compound_value(cinit, cur_init_slot++, get_entity_initializer(md_ent));
	}

	assert (cur_init_slot == n_methods);

	set_type_size_bytes(mt_type, n_methods * MD_SIZE_BYTES);
	default_layout_compound_type(mt_type);

	ir_entity        *mt_ent        = new_entity(get_glob_type(), id, mt_type);
	set_entity_initializer(mt_ent, cinit);
	set_entity_ld_ident(mt_ent, id);

	return mt_ent;
}

static ir_entity *emit_interface_table(ir_type *klass, int n_interfaces)
{
	ident            *id            = id_unique("_IF_%u_");
	ir_type          *if_type       = new_type_struct(id_mangle(id, new_id_from_str("type")));

	int               n_supertypes  = get_class_n_supertypes(klass);

	ir_initializer_t *cinit         = create_initializer_compound(n_interfaces);
	int               cur_init_slot = 0;

	for (int i = 0; i < n_supertypes; i++) {
		ir_type *iface = get_class_supertype(klass, i);
		if (! oo_get_class_is_interface(iface))
			continue;

		ir_entity *ci = oo_get_class_rtti_entity(iface);
		ir_entity *entry_ent = emit_primitive_member(if_type, "entry", type_reference, create_ccode_symconst(ci));
		set_initializer_compound_value(cinit, cur_init_slot++, get_entity_initializer(entry_ent));
	}

	assert (cur_init_slot == n_interfaces);

	set_type_size_bytes(if_type, n_interfaces * get_type_size_bytes(type_reference));
	default_layout_compound_type(if_type);

	ir_entity        *if_ent        = new_entity(get_glob_type(), id, if_type);
	set_entity_initializer(if_ent, cinit);
	set_entity_ld_ident(if_ent, id);

	return if_ent;
}

void rtti_default_construct_runtime_typeinfo(ir_type *klass)
{
	#define NUM_FIELDS 6
	#define EMIT_PRIM(name, tp, val) do { \
		ir_entity        *ent  = emit_primitive_member(ci_type, name, tp, val); \
		ir_initializer_t *init = get_entity_initializer(ent); \
		set_initializer_compound_value(ci_init, cur_init_slot++, init); \
		ir_type          *tp   = get_entity_type(ent); \
		cur_type_size         += get_type_size_bytes(tp); \
	} while(0);

	assert (is_Class_type(klass));
	ir_entity        *ci = oo_get_class_rtti_entity(klass);
	assert (ci && "RTTI entity not set. Please create and set such an entity yourself. A primitive pointer type is fine");

	ident            *cname_id = get_class_ident(klass);
	ir_type          *ci_type = new_type_struct(id_mangle_dot(cname_id, new_id_from_str("class$")));
	ir_initializer_t *ci_init = create_initializer_compound(NUM_FIELDS);
	unsigned cur_init_slot = 0;
	unsigned cur_type_size = 0;

	ir_graph *ccode_irg = get_const_code_irg();
	ir_node *const_0 = new_r_Const_long(ccode_irg, mode_P, 0);

	ir_entity *cname_ent = rtti_emit_string_const(get_id_str(cname_id));
	ir_node   *cname_symc = create_ccode_symconst(cname_ent);
	EMIT_PRIM("name", type_reference, cname_symc);

	ir_node *superclass_symc = NULL;
	int n_superclasses = get_class_n_supertypes(klass);
	if (n_superclasses > 0) {
		ir_type *superclass = get_class_supertype(klass, 0);
		if (! oo_get_class_is_interface(superclass)) {
			ir_entity *superclass_rtti = oo_get_class_rtti_entity(superclass);
			assert (superclass_rtti);
			superclass_symc = create_ccode_symconst(superclass_rtti);
		}
	}
	EMIT_PRIM("superclass", type_reference, superclass_symc ? superclass_symc : const_0);

	int n_methods = 0;
	int n_members = get_class_n_members(klass);
	for (int i = 0; i < n_members; i++) {
		ir_entity *member = get_class_member(klass, i);
		if (is_method_entity(member) && ! oo_get_method_exclude_from_vtable(member)) {
			n_methods++;
		}
	}
	ir_node *n_methods_const = new_r_Const_long(ccode_irg, mode_Is, n_methods);
	EMIT_PRIM("n_methods", type_int, n_methods_const);

	if (n_methods > 0) {
		ir_entity *method_table = emit_method_table(klass, n_methods);
		ir_node *method_table_symc = create_ccode_symconst(method_table);
		EMIT_PRIM("methods", type_reference, method_table_symc);
	} else {
		EMIT_PRIM("methods", type_reference, const_0);
	}

	int n_interfaces = 0;
	int n_supertypes = get_class_n_supertypes(klass);
	for (int i = 0; i < n_supertypes; i++) {
		ir_type *stype = get_class_supertype(klass, i);
		if (oo_get_class_is_interface(stype)) n_interfaces++;
	}
	ir_node *n_interfaces_const = new_r_Const_long(ccode_irg, mode_Is, n_interfaces);
	EMIT_PRIM("n_interfaces", type_int, n_interfaces_const);

	if (n_interfaces > 0) {
		ir_entity *iface_table = emit_interface_table(klass, n_interfaces);
		ir_node *iface_table_symc = create_ccode_symconst(iface_table);
		EMIT_PRIM("interfaces", type_reference, iface_table_symc);
	} else {
		EMIT_PRIM("interfaces", type_reference, const_0);
	}

	assert (cur_init_slot == NUM_FIELDS);

	set_type_size_bytes(ci_type, cur_type_size);
	default_layout_compound_type(ci_type);

	set_entity_type(ci, ci_type);
	set_entity_initializer(ci, ci_init);
	set_entity_visibility(ci, ir_visibility_default);
	set_entity_alignment(ci, 32);
}

ir_node *rtti_default_construct_instanceof(ir_node *objptr, ir_type *klass, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node    *cur_mem      = *mem;

	// we need the reference to the object's class$ field
	// first, dereference the vptr in order to get the vtable address.
	ir_entity  *vptr_entity  = oo_get_class_vptr_entity(klass); // XXX: this is a bit weird and works iff the vptr entity is the same in the whole type hierarchy.
	ir_node    *vptr_addr    = new_r_Sel(block, new_r_NoMem(irg), objptr, 0, NULL, vptr_entity);
	ir_node    *vptr_load    = new_r_Load(block, cur_mem, vptr_addr, mode_P, cons_none);
	ir_node    *vtable_addr  = new_r_Proj(vptr_load, mode_P, pn_Load_res);
	            cur_mem      = new_r_Proj(vptr_load, mode_M, pn_Load_M);

	// second, dereference vtable_addr+index_of_rtti_ptr.
	ir_node    *obj_ci_offset= new_r_Const_long(irg, mode_P, get_type_size_bytes(type_reference) * ddispatch_get_index_of_rtti_ptr());
	ir_node    *obj_ci_add   = new_r_Add(block, vtable_addr, obj_ci_offset, mode_P);
	ir_node    *obj_ci_load  = new_r_Load(block, cur_mem, obj_ci_add, mode_P, cons_none);
	ir_node    *obj_ci_ref   = new_r_Proj(obj_ci_load, mode_P, pn_Load_res);
	            cur_mem      = new_r_Proj(obj_ci_load, mode_M, pn_Load_M);

	// get a symconst to klass' classinfo.
	ir_entity  *test_ci      = oo_get_class_rtti_entity(klass);
	assert (test_ci);
	symconst_symbol test_ci_sym;
	test_ci_sym.entity_p = test_ci;
	ir_node   *test_ci_ref   = new_r_SymConst(irg, mode_P, test_ci_sym, symconst_addr_ent);

	symconst_symbol callee_sym;
	callee_sym.entity_p      = default_instanceof_entity;
	ir_node   *callee        = new_r_SymConst(irg, mode_P, callee_sym, symconst_addr_ent);
	ir_node   *args[2]       = { obj_ci_ref, test_ci_ref };
	ir_type   *call_type     = get_entity_type(default_instanceof_entity);
	ir_node   *call          = new_r_Call(block, cur_mem, callee, 2, args, call_type);
	           cur_mem       = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node   *ress          = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node   *res           = new_r_Proj(ress, mode_Is, 0);

	*mem = cur_mem;

	return res;
}

void rtti_init()
{
	mode_uint16_t  = new_ir_mode("US", irms_int_number, 16, 0, irma_twos_complement, 16);
	type_uint16_t  = new_type_primitive(mode_uint16_t);
	type_reference = new_type_primitive(mode_P);
	type_int       = new_type_primitive(mode_Is);

	rtti_model.construct_runtime_typeinfo = rtti_default_construct_runtime_typeinfo;
	rtti_model.construct_instanceof = rtti_default_construct_instanceof;

	ir_type *default_io_type = new_type_method(2, 1);
	set_method_param_type(default_io_type, 0, type_reference);
	set_method_param_type(default_io_type, 1, type_reference);
	set_method_res_type(default_io_type, 0, type_int);
	ident *default_io_ident = new_id_from_str("oo_rt_instanceof");
	default_instanceof_entity = new_entity(get_glob_type(), default_io_ident, default_io_type);
	set_entity_visibility(default_instanceof_entity, ir_visibility_external);

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
	(*rtti_model.construct_runtime_typeinfo)(klass);
}

void rtti_lower_InstanceOf(ir_node *instanceof)
{
	ir_node  *objptr  = get_InstanceOf_objptr(instanceof);
	ir_type  *type    = get_InstanceOf_type(instanceof);
	ir_node  *block   = get_nodes_block(instanceof);
	ir_graph *irg     = get_irn_irg(instanceof);
	ir_node  *cur_mem = get_InstanceOf_mem(instanceof);
	ir_node  *res     = (*rtti_model.construct_instanceof)(objptr, type, irg, block, &cur_mem);
	ir_node  *zero    = new_r_Const_long(irg, mode_Is, 0);
	ir_node  *cmp     = new_r_Cmp(block, res, zero);
	ir_node  *proj    = new_r_Proj(cmp, mode_b, pn_Cmp_Ne);

	turn_into_tuple(instanceof, pn_InstanceOf_max);
	set_irn_n(instanceof, pn_InstanceOf_M, cur_mem);
	set_irn_n(instanceof, pn_InstanceOf_res, proj);
}

void rtti_set_runtime_typeinfo_constructor(construct_runtime_typeinfo_t func)
{
	assert (func);
	rtti_model.construct_runtime_typeinfo = func;
}

void rtti_set_instanceof_constructor(construct_instanceof_t func)
{
	assert (func);
	rtti_model.construct_instanceof = func;
}
