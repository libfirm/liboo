#include "config.h"

#include "liboo/oo.h"

#include <assert.h>
#include "liboo/rtti.h"
#include "liboo/dmemory.h"
#include "liboo/mangle.h"
#include "adt/obst.h"
#include "adt/error.h"

typedef enum {
	k_oo_BAD = k_ir_max+1,
	k_oo_type_info,
	k_oo_entity_info,
} oo_info_kind;

typedef struct {
	oo_info_kind  kind;
	ir_entity    *vptr;
	ir_entity    *rtti;
	bool          omit_vtable;
	void         *link;
} oo_type_info;

typedef struct {
	oo_info_kind      kind;
	bool              exclude_from_vtable;
	int               vtable_index;
	bool              is_abstract;
	bool              is_constructor;
	ddispatch_binding binding;
	ir_type          *alt_namespace;
	void             *link;
} oo_entity_info;

static struct obstack oo_info_obst;

static oo_type_info *get_type_info(ir_type *type)
{
	oo_type_info *ti = (oo_type_info*) get_type_link(type);
	if (ti == NULL) {
		ti = (oo_type_info*) obstack_alloc(&oo_info_obst, sizeof(oo_type_info));
		memset(ti, 0, sizeof(*ti));
		ti->kind = k_oo_type_info;
		set_type_link(type, ti);
	} else {
		assert (ti->kind == k_oo_type_info);
	}
	return ti;
}

static oo_entity_info *get_entity_info(ir_entity *entity)
{
	oo_entity_info *ei = (oo_entity_info*) get_entity_link(entity);
	if (ei == NULL) {
		ei = (oo_entity_info*) obstack_alloc(&oo_info_obst, sizeof(oo_entity_info));
		memset(ei, 0, sizeof(*ei));
		ei->kind = k_oo_entity_info;
		set_entity_link(entity, ei);
	} else {
		assert (ei->kind == k_oo_entity_info);
	}
	return ei;
}

bool oo_get_class_omit_vtable(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	return ti->omit_vtable;
}
void oo_set_class_omit_vtable(ir_type *classtype, bool omit_vtable)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->omit_vtable = omit_vtable;
}

ir_entity *oo_get_class_vptr_entity(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	if (ti->vptr != NULL)
		return ti->vptr;

	/* recursively search for vptr entity in supertypes */
	int n_supertypes = get_class_n_supertypes(classtype);
	for (int i = 0; i < n_supertypes; ++i) {
		ir_type *supertype = get_class_supertype(classtype, i);
		ti = get_type_info(supertype);
		if (ti->vptr != NULL)
			return ti->vptr;
	}
	return NULL;
}
void oo_set_class_vptr_entity(ir_type *classtype, ir_entity *vptr)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->vptr = vptr;
}

ir_entity *oo_get_class_rtti_entity(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	return ti->rtti;
}

void  oo_set_class_rtti_entity(ir_type *classtype, ir_entity *rtti)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->rtti = rtti;
}

void *oo_get_type_link(ir_type *type)
{
	oo_type_info *ti = get_type_info(type);
	return ti->link;
}
void oo_set_type_link(ir_type *type, void* link)
{
	oo_type_info *ti = get_type_info(type);
	ti->link = link;
}

bool oo_get_method_exclude_from_vtable(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->exclude_from_vtable;
}
void oo_set_method_exclude_from_vtable(ir_entity *method, bool exclude_from_vtable)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->exclude_from_vtable = exclude_from_vtable;
}

int  oo_get_method_vtable_index(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->vtable_index;
}
void oo_set_method_vtable_index(ir_entity *method, int vtable_index)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->vtable_index = vtable_index;
}

bool oo_get_method_is_abstract(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->is_abstract;
}
void oo_set_method_is_abstract(ir_entity *method, bool is_abstract)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->is_abstract = is_abstract;
}

bool oo_get_method_is_constructor(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->is_constructor;
}
void oo_set_method_is_constructor(ir_entity *method, bool is_constructor)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->is_constructor = is_constructor;
}

ir_type *oo_get_entity_alt_namespace(ir_entity *entity)
{
	oo_entity_info *ei = get_entity_info(entity);
	return ei->alt_namespace;
}
void oo_set_entity_alt_namespace(ir_entity *entity, ir_type *namespace)
{
	oo_entity_info *ei = get_entity_info(entity);
	ei->alt_namespace = namespace;
}

ddispatch_binding oo_get_entity_binding(ir_entity *entity)
{
	oo_entity_info *ei = get_entity_info(entity);
	return ei->binding;
}
void oo_set_entity_binding(ir_entity *entity, ddispatch_binding binding)
{
	oo_entity_info *ei = get_entity_info(entity);
	ei->binding = binding;
}

void *oo_get_entity_link(ir_entity *entity)
{
	oo_entity_info *ei = get_entity_info(entity);
	return ei->link;
}
void oo_set_entity_link(ir_entity *entity, void* link)
{
	oo_entity_info *ei = get_entity_info(entity);
	ei->link = link;
}

static void setup_vtable_proxy(ir_type *klass, void *env)
{
	(void) env;
	ddispatch_setup_vtable(klass);
}

static void construct_runtime_typeinfo_proxy(ir_type *klass, void *env)
{
	(void) env;
	rtti_construct_runtime_typeinfo(klass);
}

static void lower_node(ir_node *node, void *env)
{
	(void) env;
	if (is_Alloc(node)) {
		dmemory_lower_Alloc(node);
	} else if (is_Call(node)) {
		ddispatch_lower_Call(node);
	} else if (is_Arraylength(node)) {
		dmemory_lower_Arraylength(node);
	} else if (is_InstanceOf(node)) {
		rtti_lower_InstanceOf(node);
	}
}

static void lower_type(type_or_ent tore, void *env)
{
	(void) env;
	if (get_kind(tore.typ) != k_type)
		return;

	ir_type *type = tore.typ;
	if (!is_Class_type(type)) {
		set_type_state(type, layout_fixed);
		return;
	}

	/* mangle entity names */
	int n_members = get_class_n_members(type);
	for (int m = 0; m < n_members; ++m) {
		ir_entity *entity = get_class_member(type, m);
		/* don't mangle names of entities with explicitely set names */
		if (entity_has_ld_ident(entity))
			continue;

		ident *mangled_id = mangle_entity_name(entity);
		set_entity_ld_ident(entity, mangled_id);
	}

	ir_type *glob = get_glob_type();
	if (type != glob) {
		for (int m = n_members-1; m >= 0; m--) {
			ir_entity *entity = get_class_member(type, m);
			if (is_method_entity(entity))
				set_entity_owner(entity, glob);
		}
	}

	/* layout fields */
	default_layout_compound_type(type);
}

void oo_init(void)
{
	obstack_init(&oo_info_obst);
	ddispatch_init();
	dmemory_init();
	mangle_init();
	rtti_init();
}

void oo_deinit(void)
{
	mangle_deinit();
	obstack_free(&oo_info_obst, NULL);
}

void oo_lower(void)
{
	class_walk_super2sub(setup_vtable_proxy, NULL, NULL);
	class_walk_super2sub(construct_runtime_typeinfo_proxy, NULL, NULL);

	int n_irgs = get_irp_n_irgs();
	for (int i = 0; i < n_irgs; ++i) {
		ir_graph *irg = get_irp_irg(i);
		irg_walk_graph(irg, NULL, lower_node, NULL);
	}

	type_walk_super2sub(lower_type, NULL, NULL);
}
