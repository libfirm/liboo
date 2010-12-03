#include <liboo/oo.h>

#include <assert.h>
#include "adt/obst.h"
#include "adt/error.h"

typedef enum {
	k_oo_BAD = k_ir_max+1,
	k_oo_type_info,
	k_oo_entity_info,
} oo_info_kind;

typedef struct {
	oo_info_kind kind;
	ir_entity **vptr;
	bool needs_vtable;
	void *link;
} oo_type_info;

typedef struct {
	oo_info_kind kind;
	bool include_in_vtable;
	bool is_abstract;
	ddispatch_binding binding;
	void *link;
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

bool get_class_needs_vtable(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	return ti->needs_vtable;
}
void set_class_needs_vtable(ir_type *classtype, bool needs_vtable)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->needs_vtable = needs_vtable;
}

ir_entity **get_class_vptr_entity_ptr(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	return ti->vptr;
}
void set_class_vptr_entity_ptr(ir_type *classtype, ir_entity **vptr)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->vptr = vptr;
}

void *get_oo_type_link(ir_type *type)
{
	oo_type_info *ti = get_type_info(type);
	return ti->link;
}
void set_oo_type_link(ir_type *type, void* link)
{
	oo_type_info *ti = get_type_info(type);
	ti->link = link;
}

bool get_method_include_in_vtable(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->include_in_vtable;
}
void set_method_include_in_vtable(ir_entity *method, bool include_in_vtable)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->include_in_vtable = include_in_vtable;
}

bool get_method_is_abstract(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->is_abstract;
}
void set_method_is_abstract(ir_entity *method, bool is_abstract)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->is_abstract = is_abstract;
}

ddispatch_binding get_method_binding(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->binding;
}
void set_method_binding(ir_entity *method, ddispatch_binding binding)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->binding = binding;
}

void *get_oo_entity_link(ir_entity *entity)
{
	oo_entity_info *ei = get_entity_info(entity);
	return ei->link;
}
void set_oo_entity_link(ir_entity *entity, void* link)
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
	}
}

static void move_to_global(ir_entity *entity)
{
	/* move to global type */
	ir_type *owner = get_entity_owner(entity);
	assert(is_Class_type(owner));
	ir_type *global_type = get_glob_type();
	set_entity_owner(entity, global_type);
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

	ir_type *global_type = get_glob_type();
	if (type == global_type)
		return;

	/* mangle entity names */
	int n_members = get_class_n_members(type);
	for (int m = 0; m < n_members; ++m) {
		ir_entity *entity = get_class_member(type, m);
		/* don't mangle names of entities with explicitely set names */
		if (entity_has_ld_ident(entity)) {
			continue;
		}
		ident *mangled_id = mangle_entity_name(entity);
		set_entity_ld_ident(entity, mangled_id);
	}

	n_members = get_class_n_members(type);
	for (int m = n_members-1; m >= 0; --m) {
		ir_entity *entity = get_class_member(type, m);
		if (is_method_entity(entity) ||
				get_entity_allocation(entity) == allocation_static) {
			move_to_global(entity);
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

void lower_oo(void)
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
