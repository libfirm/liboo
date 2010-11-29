#include <liboo/oo.h>

#include <libfirm/firm.h>
#include <assert.h>
#include "adt/error.h"

static void setup_vtable_proxy(ir_type *klass, void *env)
{
	(void) env;
	ddispatch_setup_vtable(klass);
}

static void construct_runtime_classinfo_proxy(ir_type *klass, void *env)
{
	(void) env;
	ddispatch_construct_runtime_classinfo(klass);
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
	ddispatch_init();
	dmemory_init();
	mangle_init();
}

void oo_deinit(void)
{
	mangle_deinit();
}

void lower_oo(void)
{
	class_walk_super2sub(setup_vtable_proxy, NULL, NULL);
	class_walk_super2sub(construct_runtime_classinfo_proxy, NULL, NULL);

	int n_irgs = get_irp_n_irgs();
	for (int i = 0; i < n_irgs; ++i) {
		ir_graph *irg = get_irp_irg(i);
		irg_walk_graph(irg, NULL, lower_node, NULL);
	}

	type_walk_super2sub(lower_type, NULL, NULL);
}
