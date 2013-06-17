#include "config.h"

#include "liboo/oo.h"

#include <assert.h>
#include "liboo/rtti.h"
#include "liboo/dmemory.h"
#include "liboo/nodes.h"
#include "liboo/eh.h"
#include "adt/obst.h"
#include "libfirm/adt/pmap.h"
#include "adt/error.h"
#include "gen_irnode.h"

typedef enum {
	oo_is_abstract  = 1 << 0,
	oo_is_final     = 1 << 1,
	oo_is_interface = 1 << 2,
	oo_is_inherited = 1 << 3,
	oo_is_extern    = 1 << 4,
	oo_is_transient = 1 << 5
} oo_info_flags;

typedef enum {
	k_oo_BAD = k_ir_max+1,
	k_oo_type_info,
	k_oo_entity_info,
	k_oo_node_info,
} oo_info_kind;

typedef struct {
	oo_info_kind  kind;
	unsigned      uid;
	ir_entity    *vptr;
	ir_entity    *rtti;
	ir_entity    *vtable;
	unsigned      vtable_size;
	unsigned      flags;
	void         *link;
} oo_type_info;

typedef struct {
	oo_info_kind      kind;
	bool              exclude_from_vtable;
	int               vtable_index;
	unsigned          flags;
	ddispatch_binding binding;
	void             *link;
} oo_entity_info;

typedef struct {
	oo_info_kind kind;
	bool         bind_statically;
} oo_node_info;

static struct obstack  oo_info_obst;
static pmap           *oo_node_info_map = NULL;

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
		ei->vtable_index = -1;
		set_entity_link(entity, ei);
	} else {
		assert (ei->kind == k_oo_entity_info);
	}
	return ei;
}

static oo_node_info *get_node_info(ir_node *node)
{
	oo_node_info *ni = pmap_get(oo_node_info, oo_node_info_map, node);
	if (ni == NULL) {
		ni = (oo_node_info*) obstack_alloc(&oo_info_obst, sizeof(oo_node_info));
		memset(ni, 0, sizeof(*ni));
		ni->kind = k_oo_node_info;
		ni->bind_statically = false;
		pmap_insert(oo_node_info_map, node, ni);
	} else {
		assert (ni->kind == k_oo_node_info);
	}
	return ni;
}

unsigned oo_get_class_uid(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	return ti->uid;
}
void oo_set_class_uid(ir_type *classtype, unsigned uid)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->uid = uid;
}

ir_entity *oo_get_class_vtable_entity(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	return ti->vtable;
}
void oo_set_class_vtable_entity(ir_type *classtype, ir_entity *vtable)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->vtable = vtable;
}

unsigned oo_get_class_vtable_size(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (ti->vtable_size == get_class_vtable_size(classtype) && "liboo and core Firm info differs");
#endif

	return ti->vtable_size;
}
void oo_set_class_vtable_size(ir_type *classtype, unsigned vtable_size)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->vtable_size = vtable_size;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_class_vtable_size(classtype, vtable_size);
#endif
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

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (ti->rtti == get_class_type_info(classtype) && "liboo and core Firm info differs");
#endif

	return ti->rtti;
}

void  oo_set_class_rtti_entity(ir_type *classtype, ir_entity *rtti)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	ti->rtti = rtti;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_class_type_info(classtype, rtti);
#endif
}

bool oo_get_class_is_interface(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);

	bool res = ti->flags & oo_is_interface;
#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (res == (bool) is_class_interface(classtype) && "liboo and core Firm info differs");
#endif

	return res;
}
void oo_set_class_is_interface(ir_type *classtype, bool is_interface)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	if (is_interface)
		ti->flags |= oo_is_interface;
	else
		ti->flags &= ~oo_is_interface;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_class_interface(classtype, is_interface);
#endif
}

bool oo_get_class_is_abstract(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);

	bool res = ti->flags & oo_is_abstract;
#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (res == (bool) is_class_abstract(classtype) && "liboo and core Firm info differs");
#endif

	return res;
}
void oo_set_class_is_abstract(ir_type *classtype, bool is_abstract)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	if (is_abstract)
		ti->flags |= oo_is_abstract;
	else
		ti->flags &= ~oo_is_abstract;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_class_abstract(classtype, is_abstract);
#endif
}

bool oo_get_class_is_final(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);

	bool res = ti->flags & oo_is_final;
#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (res == (bool) is_class_final(classtype) && "liboo and core Firm info differs");
#endif

	return res;
}
void oo_set_class_is_final(ir_type *classtype, bool is_final)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	if (is_final)
		ti->flags |= oo_is_final;
	else
		ti->flags &= ~oo_is_final;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_class_final(classtype, is_final);
#endif
}

bool oo_get_class_is_extern(ir_type *classtype)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);

	return ti->flags & oo_is_extern;
}
void oo_set_class_is_extern(ir_type *classtype, bool is_extern)
{
	assert (is_Class_type(classtype));
	oo_type_info *ti = get_type_info(classtype);
	if (is_extern)
		ti->flags |= oo_is_extern;
	else
		ti->flags &= ~oo_is_extern;

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

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (ei->vtable_index == (int) get_entity_vtable_number(method) && "liboo and core Firm info differs");
#endif

	return ei->vtable_index;
}
void oo_set_method_vtable_index(ir_entity *method, int vtable_index)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	ei->vtable_index = vtable_index;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_entity_vtable_number(method, vtable_index);
#endif
}

bool oo_get_method_is_abstract(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	return ei->flags & oo_is_abstract;
}
void oo_set_method_is_abstract(ir_entity *method, bool is_abstract)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	if (is_abstract)
		ei->flags |= oo_is_abstract;
	else
		ei->flags &= ~oo_is_abstract;
}

bool oo_get_method_is_final(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	bool res = ei->flags & oo_is_final;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (res == (bool) is_entity_final(method) && "liboo and core Firm info differs");
#endif

	return res;
}
void oo_set_method_is_final(ir_entity *method, bool is_final)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	if (is_final)
		ei->flags |= oo_is_final;
	else
		ei->flags &= ~oo_is_final;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_entity_final(method, is_final);
#endif
}

bool oo_get_method_is_inherited(ir_entity *method)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	bool res = ei->flags & oo_is_inherited;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	assert (res == (get_entity_peculiarity(method) == peculiarity_inherited) && "liboo and core Firm info differs");
#endif

	return res;
}
void oo_set_method_is_inherited(ir_entity *method, bool is_inherited)
{
	assert (is_method_entity(method));
	oo_entity_info *ei = get_entity_info(method);
	if (is_inherited)
		ei->flags |= oo_is_inherited;
	else
		ei->flags &= ~oo_is_inherited;

#ifdef OO_ALSO_USE_OLD_FIRM_PROPERTIES
	set_entity_peculiarity(method, is_inherited ? peculiarity_inherited : peculiarity_existent);
#endif
}

bool oo_get_field_is_transient(ir_entity *field)
{
	assert (! is_method_entity(field));
	oo_entity_info *ei = get_entity_info(field);
	return ei->flags & oo_is_transient;
}
void oo_set_field_is_transient(ir_entity *field, bool is_transient)
{
	assert (! is_method_entity(field));
	oo_entity_info *ei = get_entity_info(field);
	if (is_transient)
		ei->flags |= oo_is_transient;
	else
		ei->flags &= ~oo_is_transient;
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

void oo_set_call_is_statically_bound(ir_node *call, bool bind_statically)
{
	assert (is_Call(call));
	oo_node_info *ni = get_node_info(call);
	ni->bind_statically = bind_statically;
}

bool oo_get_call_is_statically_bound(ir_node *call)
{
	assert (is_Call(call));

	/* Shortcut to avoid creating a node_info for each call we see.
	 * Nearly all calls are bound just like their referenced method
	 * entity specifies, and just very few (like super.method() in
	 * Java) must be handled specially. */
	if (! pmap_contains(oo_node_info_map, call))
		return false;

	oo_node_info *ni = get_node_info(call);
	return ni->bind_statically;
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

// Filter out supertypes that are interfaces and return the first real superclass.
ir_type *oo_get_class_superclass(ir_type *klass)
{
	assert (is_Class_type(klass));

	ir_type *superclass = NULL;
	size_t n_supertyes = get_class_n_supertypes(klass);
	for (size_t s = 0; s < n_supertyes; s++) {
		ir_type *st = get_class_supertype(klass, s);
		if (oo_get_class_is_interface(st))
			continue;

		assert (! superclass && "multiple inheritance unsupported");

		superclass = st;
	}
	return superclass;
}

// Filter out the overwritten entites that belong to interfaces.
ir_entity *oo_get_entity_overwritten_superclass_entity(ir_entity *entity)
{
	assert (is_method_entity(entity));

	ir_entity *superclass_entity = NULL;
	size_t n_overwrites = get_entity_n_overwrites(entity);
	for (size_t s = 0; s < n_overwrites; s++) {
		ir_entity *se = get_entity_overwrites(entity, s);
		ir_type *owner = get_entity_owner(se);
		assert (owner != get_glob_type());
		if (oo_get_class_is_interface(owner))
			continue;

		assert (! superclass_entity && "multiple inheritance unsupported");

		superclass_entity = se;
	}
	return superclass_entity;
}

void oo_copy_entity_info(ir_entity *src, ir_entity *dest)
{
	assert (is_entity(src) && is_entity(dest));
	oo_entity_info *ei_src  = get_entity_info(src);
	oo_entity_info *ei_dest = get_entity_info(dest);

	memcpy(ei_dest, ei_src, sizeof(oo_entity_info));
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
	if (is_Call(node)) {
		ddispatch_lower_Call(node);
	} else if (is_Arraylength(node)) {
		dmemory_lower_Arraylength(node);
	} else if (is_InstanceOf(node)) {
		rtti_lower_InstanceOf(node);
	}
	/*else if (is_Raise(node)) {
		eh_lower_Raise(node);
	}*/
}

static void lower_type(ir_type *type, void *env)
{
	(void) env;
	assert (is_Class_type(type));

	ir_type *glob = get_glob_type();
	if (type != glob) {
		int n_members = get_class_n_members(type);
		for (int m = n_members-1; m >= 0; m--) {
			ir_entity *entity = get_class_member(type, m);
			if (is_method_entity(entity))
				set_entity_owner(entity, glob);
		}
	}
}

void oo_init(void)
{
	obstack_init(&oo_info_obst);
	oo_node_info_map = pmap_create();
	oo_init_opcodes();
	ddispatch_init();
	dmemory_init();
	rtti_init();
	eh_init();
}

void oo_deinit(void)
{
	rtti_deinit();
	eh_deinit();
	obstack_free(&oo_info_obst, NULL);
	pmap_destroy(oo_node_info_map);
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

	class_walk_super2sub(lower_type, NULL, NULL);
}
