#include "liboo/ddispatch.h"
#include "liboo/oo.h"
#include "liboo/rtti.h"
#include "liboo/dmemory.h"
#include "liboo/nodes.h"

#include <assert.h>
#include "adt/error.h"
#include "adt/util.h"
#include "adt/obst.h"
#include "adt/cpset.h"
#include "adt/cpmap.h"
#include "adt/hashptr.h"

#include <libfirm/ident.h>

static ir_mode   *mode_reference;
static ir_type   *type_reference;
static ir_mode   *mode_int;
static ir_type   *type_int;
static ir_type   *type_char;

static ir_type *itt_entry_type = NULL;

static ir_entity *default_lookup_interface_entity;
static ir_entity *searched_itable_interface_entity;

struct ddispatch_model_t {
	unsigned                      vptr_points_to_index;
	unsigned                      index_of_rtti_ptr;
	unsigned                      index_of_itt_ptr;
	unsigned                      index_of_first_method;
	init_vtable_slots_t           init_vtable_slots;
	ident                        *abstract_method_ident;
	construct_interface_lookup_t  construct_interface_lookup;
} ddispatch_model;

typedef struct ddispatch_klass_iterator_t ddispatch_klass_iterator_t;
struct ddispatch_klass_iterator_t {
	ddispatch_klass_iterator_t *next;
	ddispatch_klass_iterator_t *head;
	ir_type *klass;
	bool exists;
};

typedef struct ddispatch_method_iterator_t ddispatch_method_iterator_t;
struct ddispatch_method_iterator_t {
	ddispatch_method_iterator_t *next;
	ddispatch_method_iterator_t *head;
	ir_entity *method;
	bool exists;
};

static struct obstack ddispatch_obst;

static cpmap_t interface_index_map;
static cpmap_t it_index_map;
static unsigned interface_index;

typedef struct {
	ir_type   *interface;
	ir_entity *comdat_node;
	unsigned   index;
} interface_index_entry;

typedef struct {
	ir_type   *interface;
	ir_entity *method;
	int        index;
} it_index_map_entry;

static interface_index_entry *get_itt_entry(ir_type *klass)
{
	return cpmap_find(&interface_index_map, klass);
}

static void *combine_ptr_hash(void *p1, void *p2)
{
	return (void*)(size_t)HASH_COMBINE(hash_ptr(p1), hash_ptr(p2));
}

int ddispatch_get_itable_method_index(ir_type *interface, ir_entity *method)
{
	it_index_map_entry *found_itie = cpmap_find(&it_index_map, combine_ptr_hash(interface, method));
	if (found_itie != NULL) {
		return found_itie->index;
	}

	return -1;
}

ir_entity *ddispatch_get_itable_id(ir_type *interface)
{
	interface_index_entry *entry = get_itt_entry(interface);
	return entry->comdat_node;
}

ir_type *ddispatch_get_itt_entry_type()
{
	if (itt_entry_type == NULL) {
		itt_entry_type = new_type_struct(new_id_from_str("itable_table_entry"));

		new_entity(itt_entry_type, new_id_from_str("itable"), type_reference);
		if ((oo_get_interface_call_type() & call_searched_itable) == call_searched_itable) {
			// Only needed for searched itables
			new_entity(itt_entry_type, new_id_from_str("id"), type_reference);
			new_entity(itt_entry_type, new_id_from_str("prev"), type_int);
			new_entity(itt_entry_type, new_id_from_str("next"), type_int);
		}
		default_layout_compound_type(itt_entry_type);
	}

	return itt_entry_type;
}

unsigned ddispatch_get_itable_index(ir_type *interface)
{
	interface_index_entry *entry = get_itt_entry(interface);
	return entry->index;
}

static void add_itable_method_index(ir_type *interface, ir_entity *method, int index)
{
	if (ddispatch_get_itable_method_index(interface, method) != -1) {
		return;
	}

	it_index_map_entry *new_itie = OALLOC(&ddispatch_obst, it_index_map_entry);
	new_itie->interface = interface;
	new_itie->method = method;
	new_itie->index = index;

	cpmap_set(&it_index_map, combine_ptr_hash(interface, method), new_itie);
}

static int ptr_equals(const void *p1, const void *p2)
{
	return p1 == p2;
}

static void default_init_vtable_slots(ir_type* klass, ir_initializer_t *vtable_init, unsigned vtable_size)
{
	(void) klass; (void) vtable_size;
	ir_graph         *ccode_irg = get_const_code_irg();

	ir_entity        *ci        = oo_get_class_rtti_entity(klass);
	assert (ci);

	ir_node          *ci_symc   = new_r_Address(ccode_irg, ci);
	ir_initializer_t *ci_init   = create_initializer_const(ci_symc);
	set_initializer_compound_value(vtable_init, ddispatch_model.index_of_rtti_ptr, ci_init);

	if (oo_get_interface_call_type() != call_runtime_lookup) {
		ir_entity        *itt       = oo_get_class_itt_entity(klass);
		assert(itt);

		ir_node          *itt_symc  = new_r_Address(ccode_irg, itt);
		ir_initializer_t *itt_init  = create_initializer_const(itt_symc);
		set_initializer_compound_value(vtable_init, ddispatch_model.index_of_itt_ptr, itt_init);
	}

	ir_node          *const_0   = new_r_Const_long(ccode_irg, mode_reference, 0);
	ir_initializer_t *slot_init = create_initializer_const(const_0);

	for (unsigned i = 0; i < ddispatch_model.index_of_first_method; i++) {
		if (i == ddispatch_model.index_of_rtti_ptr) continue;
		if (i == ddispatch_model.index_of_itt_ptr) continue;
		set_initializer_compound_value(vtable_init, i, slot_init);
	}
}

static ir_node *default_interface_lookup_method(ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node    *cur_mem        = *mem;

	// we need the reference to the object's class$ field
	// first, dereference the vptr in order to get the vtable address.
	ir_entity  *vptr_entity    = oo_get_class_vptr_entity(iface);
	ir_type    *vptr_type      = get_entity_type(vptr_entity);
	ir_node    *vptr_addr      = new_r_Member(block, objptr, vptr_entity);
	ir_node    *vptr_load      = new_r_Load(block, cur_mem, vptr_addr, mode_P, vptr_type, cons_none);
	ir_node    *vtable_addr    = new_r_Proj(vptr_load, mode_P, pn_Load_res);
	            cur_mem        = new_r_Proj(vptr_load, mode_M, pn_Load_M);

	// second, calculate the position of the RTTI ref in relation to the target of vptr and dereference it.
	int         offset         = (ddispatch_model.index_of_rtti_ptr - ddispatch_model.vptr_points_to_index) * get_type_size_bytes(type_reference);
	ir_mode    *mode_offset    = get_reference_offset_mode(mode_P);
	ir_node    *ci_offset      = new_r_Const_long(irg, mode_offset, offset);
	ir_node    *ci_add         = new_r_Add(block, vtable_addr, ci_offset, mode_P);
	ir_node    *ci_load        = new_r_Load(block, cur_mem, ci_add, mode_P, vptr_type, cons_none);
	ir_node    *ci_ref         = new_r_Proj(ci_load, mode_P, pn_Load_res);
	            cur_mem        = new_r_Proj(ci_load, mode_M, pn_Load_M);

	const char *method_name    = get_entity_name(method);
	ir_entity  *name_const_ent = rtti_emit_string_const(method_name);
	ir_node    *name_ref       = new_r_Address(irg, name_const_ent);

	ir_node   *callee        = new_r_Address(irg, default_lookup_interface_entity);

	ir_node   *args[2]       = { ci_ref, name_ref };
	ir_type   *call_type     = get_entity_type(default_lookup_interface_entity);
	ir_node   *call          = new_r_Call(block, cur_mem, callee, 2, args, call_type);
	           cur_mem       = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node   *ress          = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node   *res           = new_r_Proj(ress, mode_P, 0);

	*mem = cur_mem;

	return res;
}

static ir_node *interface_lookup_indexed(ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node *new_mem = *mem;
	ir_node *new_res;

	ir_type *classtype = get_entity_owner(method);
	assert(is_Class_type(classtype));

	int itable_id = ddispatch_get_itable_method_index(iface, method);
	if (itable_id == -1) {
		// TODO: Remove when codegen bug for generic methods with method guard is fixed
		return default_interface_lookup_method(objptr, iface, method, irg, block, mem);
	}
	assert(itable_id != -1);

	ir_entity *vptr_entity  = oo_get_class_vptr_entity(classtype);
	ir_type   *vptr_type    = get_entity_type(vptr_entity);
	ir_node   *vptr         = new_r_Member(block, objptr, vptr_entity);

	ir_type   *type_unknown = get_unknown_type();
	ir_type   *itt_type     = type_unknown;

	unsigned type_ref_size  = get_type_size_bytes(type_reference);
	ir_mode *mode_offset    = get_reference_offset_mode(mode_reference);

	// Load vtable
	ir_node   *vtable_load  = new_r_Load(block, *mem, vptr, mode_reference, vptr_type, cons_none);
	ir_node   *vtable_addr  = new_r_Proj(vtable_load, mode_reference, pn_Load_res);
	ir_node   *vtable_mem   = new_r_Proj(vtable_load, mode_M, pn_Load_M);


	// Compute and load ITT address
	ir_node   *itt_offset   = new_r_Const_long(irg, mode_offset, 1 * type_ref_size);
	ir_node   *itt_ptr_addr = new_r_Add(block, vtable_addr, itt_offset, mode_reference);

	ir_node   *itt_load     = new_r_Load(block, vtable_mem, itt_ptr_addr, mode_reference, itt_type, cons_none);
	ir_node   *itt_addr     = new_r_Proj(itt_load, mode_reference, pn_Load_res);
	ir_node   *itt_mem      = new_r_Proj(itt_load, mode_M, pn_Load_M);

	// Compute and load itable address
	unsigned   entry_size   = get_type_size_bytes(ddispatch_get_itt_entry_type());
	unsigned   it_id        = ddispatch_get_itable_index(iface);
	ir_node   *it_offset    = new_r_Const_long(irg, mode_offset, it_id * entry_size);
	ir_node   *it_ptr_addr  = new_r_Add(block, itt_addr, it_offset, mode_reference);

	ir_node   *it_load      = new_r_Load(block, itt_mem, it_ptr_addr, mode_reference, vptr_type, cons_none);
	ir_node   *it_addr      = new_r_Proj(it_load, mode_reference, pn_Load_res);
	ir_node   *it_mem       = new_r_Proj(it_load, mode_M, pn_Load_M);

	// Call method
	ir_node *vtable_offset  = new_r_Const_long(irg, mode_offset, itable_id * type_ref_size);
	ir_node *funcptr_addr   = new_r_Add(block, it_addr, vtable_offset, mode_reference);
	ir_node *callee_load    = new_r_Load(block, it_mem, funcptr_addr, mode_reference, vptr_type, cons_none);
	new_res                 = new_r_Proj(callee_load, mode_reference, pn_Load_res);
	new_mem                 = new_r_Proj(callee_load, mode_M, pn_Load_M);

	mem = &new_mem;

	return new_res;
}

static ir_node *interface_lookup_searched_itable(ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem)
{
	ir_node    *cur_mem        = *mem;

	int itable_index = ddispatch_get_itable_method_index(iface, method);
	if (itable_index == -1) {
		// TODO: Remove when codegen bug for generic methods with method guard is fixed
		return default_interface_lookup_method(objptr, iface, method, irg, block, mem);
	}
	assert(itable_index!= -1);

	ir_entity *interface_id  = ddispatch_get_itable_id(iface);

	ir_node   *callee        = new_r_Address(irg, searched_itable_interface_entity);
	ir_node   *interface_ref = new_r_Address(irg, interface_id);

	ir_mode *mode_offset     = get_reference_offset_mode(mode_reference);
	ir_node *itable_offset   = new_r_Const_long(irg, mode_offset, itable_index);

	// Call method
	ir_node   *args[3]       = { objptr, interface_ref, itable_offset};
	ir_type   *call_type     = get_entity_type(searched_itable_interface_entity);
	ir_node   *call          = new_r_Call(block, cur_mem, callee, 3, args, call_type);
	cur_mem                  = new_r_Proj(call, mode_M, pn_Call_M);
	ir_node   *ress          = new_r_Proj(call, mode_T, pn_Call_T_result);
	ir_node   *res           = new_r_Proj(ress, mode_P, 0);

	*mem = cur_mem;

	return res;
}

void ddispatch_deinit(void)
{
	obstack_free(&ddispatch_obst, NULL);

	cpmap_destroy(&interface_index_map);
	cpmap_destroy(&it_index_map);
}


void ddispatch_init(void)
{
	mode_reference = mode_P;
	type_reference = new_type_primitive(mode_reference);
	ir_mode *mode_char = new_int_mode("C", irma_twos_complement, 16, 0, 16);
	type_char = new_type_primitive(mode_char);
	mode_int = new_int_mode("I", irma_twos_complement, 32, 1, 32);
	type_int = new_type_primitive(mode_int);
	ir_mode *mode_offset = get_reference_offset_mode(mode_reference);
	ir_type *type_offset = new_type_primitive(mode_offset);

	interface_index = 0;

	cpmap_init(&interface_index_map, hash_ptr, ptr_equals);
	cpmap_init(&it_index_map, hash_ptr, ptr_equals);

	obstack_init(&ddispatch_obst);

	ddispatch_model.vptr_points_to_index        = 0;
	ddispatch_model.index_of_first_method       = 2;
	ddispatch_model.index_of_rtti_ptr           = 0;
	ddispatch_model.index_of_itt_ptr            = 1;
	ddispatch_model.init_vtable_slots           = default_init_vtable_slots;
	ddispatch_model.abstract_method_ident       = new_id_from_str("oo_rt_abstract_method_error");
	if ((oo_get_interface_call_type() & call_searched_itable) == call_searched_itable)
		ddispatch_model.construct_interface_lookup = interface_lookup_searched_itable;
	else if (oo_get_interface_call_type() == call_itable_indexed)
		ddispatch_model.construct_interface_lookup = interface_lookup_indexed;
	else
		ddispatch_model.construct_interface_lookup  = default_interface_lookup_method;

	// Default Lookup
	ir_type *default_li_type = new_type_method(2, 1);
	set_method_param_type(default_li_type, 0, type_reference);
	set_method_param_type(default_li_type, 1, type_reference);
	set_method_res_type(default_li_type, 0, type_reference);
	ident *default_li_ident = new_id_from_str("oo_rt_lookup_interface_method");
	default_lookup_interface_entity
		= create_compilerlib_entity(default_li_ident, default_li_type);

	// Searched Itable Lookup
	ir_type *si_li_type = new_type_method(3, 1);
	set_method_param_type(si_li_type, 0, type_reference);
	set_method_param_type(si_li_type, 1, type_reference);
	set_method_param_type(si_li_type, 2, type_offset);
	set_method_res_type(si_li_type, 0, type_reference);
	ident *si_li_ident;
	if ((oo_get_interface_call_type() & call_move2front) == call_move2front) {
		si_li_ident = new_id_from_str("oo_searched_itable_method_m2f");
	} else {
		si_li_ident = new_id_from_str("oo_searched_itable_method");
	}
	searched_itable_interface_entity
		= create_compilerlib_entity(si_li_ident, si_li_type);
}


static ddispatch_method_iterator_t *create_method_iterator(ir_type *klass)
{
	ddispatch_method_iterator_t *head = XMALLOC(ddispatch_method_iterator_t);
	ddispatch_method_iterator_t *current = head;

	current->head = head;
	current->exists = false;
	current->next = NULL;

	size_t n = get_class_n_members(klass);
	for (size_t i = 0; i < n; i++) {
		ir_entity *member = get_class_member(klass, i);
		if (!is_method_entity(member))
			continue;
		if (oo_get_method_exclude_from_vtable(member))
			continue;

		current->method = member;
		current->exists = true;

		current->next = XMALLOC(ddispatch_method_iterator_t);
		current = current->next;
		current->exists = false;
		current->head = head;
		current->next = NULL;
	}

	return head;
}

static ddispatch_klass_iterator_t *create_klass_iterator_rec(ir_type *klass, ddispatch_klass_iterator_t *node, cpset_t *pool)
{

	size_t n_supertyes = get_class_n_supertypes(klass);
	for (size_t s = 0; s < n_supertyes; s++) {
		ir_type *st = get_class_supertype(klass, s);

		if (cpset_find(pool, st) != NULL)
			continue;

		node->exists = true;
		node->klass = st;
		ddispatch_klass_iterator_t *temp = XMALLOC(ddispatch_klass_iterator_t);
		temp->head = node->head;
		temp->next = NULL;
		temp->exists = false;
		node->next = temp;
		node = temp;

		cpset_insert(pool, st);

		node = create_klass_iterator_rec(st, node, pool);
	}

	return node;
}

static ddispatch_klass_iterator_t *create_klass_iterator(ir_type *klass)
{
	cpset_t pool;
	cpset_init(&pool, hash_ptr, ptr_equals);

	// Dummy head - might be removed later
	ddispatch_klass_iterator_t *head = XMALLOC(ddispatch_klass_iterator_t);
	head->head = head;
	head->exists = false;
	head->next = NULL;
	create_klass_iterator_rec(klass, head, &pool);

	cpset_destroy(&pool);
	return head;
}

static void free_klass_iterator(ddispatch_klass_iterator_t *iterator)
{
	ddispatch_klass_iterator_t *current = iterator;
	ddispatch_klass_iterator_t *temp;

	if (iterator == NULL) return;
	iterator = iterator->head;

	while (current != NULL) {
		temp = current;
		current = current->next;
		free(temp);
	}
}

static void free_method_iterator(ddispatch_method_iterator_t *iterator)
{
	ddispatch_method_iterator_t *current = iterator;
	ddispatch_method_iterator_t *temp;

	if (iterator == NULL) return;
	iterator = iterator->head;

	while (current != NULL) {
		temp = current;
		current = current->next;
		free(temp);
	}
}

static ir_entity* get_method_entity(ir_type *klass, const char *entity_name)
{
	ir_entity *entity = NULL;
	ddispatch_method_iterator_t *iterator = create_method_iterator(klass);
	while (iterator->exists) {
		ir_entity *method = iterator->method;
		const char *method_name = get_entity_name(method);
		if (method_name == entity_name) {
			entity = method;
			break;
		}
		iterator = iterator->next;
	}

	free_method_iterator(iterator);
	return entity;
}

static size_t count_interface_methods(ir_type *interface)
{
	size_t itable_size = 0;

	ddispatch_klass_iterator_t *interface_iterator = create_klass_iterator(interface);

	while (interface_iterator->exists) {
		if (oo_get_class_is_interface(interface_iterator->klass)) {
			ddispatch_method_iterator_t *method_iterator = create_method_iterator(interface_iterator->klass);
			while (method_iterator->exists) {
				itable_size++;
				method_iterator = method_iterator->next;
			}
			free_method_iterator(method_iterator);
		}
		interface_iterator = interface_iterator->next;
	}

	free_klass_iterator(interface_iterator);

	ddispatch_method_iterator_t *method_iterator = create_method_iterator(interface);
	while (method_iterator->exists) {
		itable_size++;
		method_iterator = method_iterator->next;
	}
	free_method_iterator(method_iterator);

	return itable_size;
}


static ir_entity *find_method_in_hierachy(ir_entity *method, ir_type *klass)
{
	const char *method_name = get_entity_name(method);

	ir_entity *implementation = get_method_entity(klass, method_name);
	if (implementation == NULL) {
		ddispatch_klass_iterator_t *iterator = create_klass_iterator(klass);
		while (iterator->exists) {
			ir_type *st = iterator->klass;
			implementation = get_method_entity(st, method_name);
			if (implementation != NULL) break;
			iterator = iterator->next;
		}
		free_klass_iterator(iterator);
	}

	return implementation;
}

// Search klass for implementation of method and insert into itable
static void add_method_to_itable(ir_type *klass, ir_type *interface, ir_entity *method, ir_initializer_t *init, int offset)
{
	ir_entity *implementation = find_method_in_hierachy(method, klass);

	ir_graph *const_code = get_const_code_irg();
	ident *id;
	if (oo_get_method_is_abstract(implementation)) {
		id = ddispatch_model.abstract_method_ident;
	} else {
		id = get_entity_ld_ident(implementation);
	}

	ir_entity *bound = create_compilerlib_entity(id, get_entity_type(implementation));
	ir_node *symconst_node = new_r_Address(const_code, bound);
	ir_initializer_t *val = create_initializer_const(symconst_node);
	set_initializer_compound_value(init, offset, val);

	add_itable_method_index(interface, method, offset);
}

static ir_entity* create_itable(ir_type *klass, ir_type *interface) {
	size_t itable_size = count_interface_methods(interface);

	// Create itable and initializer
	ident *itable_ident = id_unique("itable_%u");
	unsigned type_reference_size = get_type_size_bytes(type_reference);
	ir_type *itable_type = new_type_array(type_reference);
	size_t itable_ent_size = itable_size;
	set_array_size_int(itable_type, itable_ent_size);
	set_type_size_bytes(itable_type, type_reference_size * itable_ent_size);
	set_type_state(itable_type, layout_fixed);

	ir_type    *unknown      = get_unknown_type();
	ir_type    *glob         = get_glob_type();
	ir_entity  *itable       = new_entity(glob, itable_ident, unknown);

	set_entity_type(itable, itable_type);
	set_entity_alignment(itable, 32);

	ir_initializer_t *init = create_initializer_compound(itable_ent_size);

	int n_method = 0;

	// Iterate over interface's parents and insert methods into itable
	ddispatch_klass_iterator_t *klass_iterator = create_klass_iterator(interface);
	while (klass_iterator->exists) {
		if (oo_get_class_is_interface(klass_iterator->klass)) {
			ddispatch_method_iterator_t *method_iterator = create_method_iterator(klass_iterator->klass);
			while (method_iterator->exists) {
				add_method_to_itable(klass, interface, method_iterator->method, init, n_method++);
				method_iterator = method_iterator->next;
			}
			free_method_iterator(method_iterator);
		}
		klass_iterator = klass_iterator->next;
	}
	free_klass_iterator(klass_iterator);

	// Iterator over interface and insert methods
	ddispatch_method_iterator_t *method_iterator = create_method_iterator(interface);
	while (method_iterator->exists) {
		add_method_to_itable(klass, interface, method_iterator->method, init, n_method++);
		method_iterator = method_iterator->next;
	}
	free_method_iterator(method_iterator);

	set_entity_initializer(itable, init);

	return itable;
}

static ir_initializer_t *create_itt(ir_type *klass, size_t size)
{
	// Create ITT and initializer
	ir_type *entry_type = ddispatch_get_itt_entry_type();
	unsigned type_reference_size = get_type_size_bytes(entry_type);
	ir_type *itt_type = new_type_array(entry_type);
	set_array_size_int(itt_type, size);
	set_type_size_bytes(itt_type, type_reference_size * size);
	set_type_state(itt_type, layout_fixed);

	ir_type    *glob         = get_glob_type();
	ir_entity  *itable_table = new_entity(glob, id_unique("itt_%u"), type_char);

	set_entity_type(itable_table, itt_type);
	set_entity_alignment(itable_table, 32);

	ir_initializer_t *initializer = create_initializer_compound(size);
	set_entity_initializer(itable_table, initializer);

	oo_set_class_itt_entity(klass, itable_table);

	return initializer;
}

static void add_start_to_itt(ir_initializer_t *initializer)
{
	ir_initializer_t *cinit;

	if ((oo_get_interface_call_type() & call_searched_itable) == call_searched_itable) {
		cinit = create_initializer_compound(4);


		// index 1/2: index of next/previous method (for move2front runtime mechanism)
		ir_node *prev_node = new_r_Const_long(
				get_const_code_irg(),
				mode_int,
				0
		);

		ir_node *next_node = new_r_Const_long(
				get_const_code_irg(),
				mode_int,
				1
		);

		set_initializer_compound_value(cinit, 0, get_initializer_null()); // it
		set_initializer_compound_value(cinit, 1, get_initializer_null()); // id
		set_initializer_compound_value(cinit, 2, create_initializer_const(prev_node)); // prev
		set_initializer_compound_value(cinit, 3, create_initializer_const(next_node)); // next
	} else {
		cinit = create_initializer_compound(1);
		set_initializer_compound_value(cinit, 0, get_initializer_null()); // it
	}
	set_initializer_compound_value(initializer, 0, cinit);
}

static void add_itable_to_itt(ir_initializer_t *initializer, ir_type *interface, ir_entity *itable, size_t offset, size_t itable_count)
{
	// Create ITT entry
	ir_initializer_t *cinit;

	if ((oo_get_interface_call_type() & call_searched_itable) == call_searched_itable) {
		cinit = create_initializer_compound(4);

		// index 1: name
		interface_index_entry *entry = get_itt_entry(interface);
		ir_node *comdat_node = new_r_Address(
				get_const_code_irg(),
				create_compilerlib_entity(
						get_entity_ld_ident(entry->comdat_node),
						get_entity_type(entry->comdat_node)));
		set_initializer_compound_value(cinit, 1, create_initializer_const(comdat_node));

		// index 2: index of next method
		ir_node *prev_node = new_r_Const_long(
				get_const_code_irg(),
				mode_int,
				offset - 1
		);
		set_initializer_compound_value(cinit, 2, create_initializer_const(prev_node));

		ir_node *next_node = new_r_Const_long(
				get_const_code_irg(),
				mode_int,
				offset >= itable_count ? 0 : offset + 1
		);
		set_initializer_compound_value(cinit, 3, create_initializer_const(next_node));
	} else {
		cinit = create_initializer_compound(1);
	}

	// index 0: ref to itable
	ir_node *address_node = new_r_Address(
			get_const_code_irg(),
			create_compilerlib_entity(
					get_entity_ld_ident(itable),
					get_entity_type(itable)));
	set_initializer_compound_value(cinit, 0, create_initializer_const(address_node));

	set_initializer_compound_value(initializer, offset, cinit);
}

static bool is_interface_empty(ir_type *interface)
{
	size_t n = get_class_n_members(interface);
	for (size_t i = 0; i < n; i++) {
		ir_entity *member = get_class_member(interface, i);
		if (!is_method_entity(member))
			continue;
		if (oo_get_method_exclude_from_vtable(member))
			continue;

		return false;
	}

	size_t n_supertyes = get_class_n_supertypes(interface);
	for (size_t s = 0; s < n_supertyes; s++) {
		ir_type *st = get_class_supertype(interface, s);

		if (oo_get_class_is_interface(st)) {
			if (!is_interface_empty(st)) {
				return false;
			}
		}
	}

	return true;
}

void ddispatch_setup_itable(ir_type *klass)
{
	assert(is_Class_type(klass));

	if (!is_Class_type(klass)) {
		return;
	}

	// Found interface => Create entry in interface_index_map and comdat node
	// that will later be used as id into ITT
	if (oo_get_class_is_interface(klass)) {
		if (is_interface_empty(klass)) {
			return;
		}

		interface_index_entry *entry = OALLOC(&ddispatch_obst, interface_index_entry);
		entry->index = interface_index++;
		entry->interface = klass;

		ir_initializer_t *init = get_initializer_null();
		ir_entity *node = new_entity(
				get_glob_type(),
				id_unique("itable_table_name_%u"),
				type_char);
		entry->comdat_node = node;
		add_entity_linkage(node, IR_LINKAGE_MERGE | IR_LINKAGE_GARBAGE_COLLECT);
		set_entity_initializer(node, init);

		cpmap_set(&interface_index_map, klass, entry);
		return;
	}

	size_t n_itable_count = 0;
	size_t itable_size = 0;
	ddispatch_klass_iterator_t *iterator = create_klass_iterator(klass);

	while (iterator->exists) {
		ir_type* st = iterator->klass;
		if (oo_get_class_is_interface(st) && !is_interface_empty(st)) {
			size_t itable_index = ddispatch_get_itable_index(st);
			n_itable_count++;
			if (itable_index + 1 > itable_size) {
				itable_size = itable_index + 1;
			}
		}

		iterator = iterator->next;
	}

	if (n_itable_count > 0) {
		size_t n_offset = 1;

		if ((oo_get_interface_call_type() & call_searched_itable) == call_searched_itable) {
			itable_size = n_itable_count + 1;
		}

		ir_initializer_t *initializer = create_itt(klass, itable_size);

		if ((oo_get_interface_call_type() & call_searched_itable) == call_searched_itable) {
			// NULL entry at end of ITT to mark end
			add_start_to_itt(initializer);
		}

		// If setup_vtable has already been called (i.e., dispatch_setup_vtable has not been executed via typewalk
		// but via finalize_class_type), ITT must be registered again (necessary for bytecode2firm)
		ir_entity *vtable = oo_get_class_vtable_entity(klass);
		if (vtable != NULL) {
			ir_initializer_t *vtable_init = get_entity_initializer(vtable);
			if (vtable_init != NULL) {
				ir_entity *itt_entity = oo_get_class_itt_entity(klass);
				ir_node *ref = new_r_Address(get_const_code_irg(), itt_entity);
				set_initializer_compound_value(
						vtable_init,
						ddispatch_model.index_of_itt_ptr,
						create_initializer_const(ref));
			}
		}

		// Recursively walks all parents of klass and generates for all interfaces I
		// an itable (klass, I)
		iterator = iterator->head;
		while (iterator->exists) {
			ir_type *st = iterator->klass;

			if (oo_get_class_is_interface(st)) {
				if (is_interface_empty(st) == false) {
					size_t itt_index = n_offset++;
					ir_entity *itable = create_itable(klass, st);

					if (oo_get_interface_call_type() == call_itable_indexed) {
						itt_index = ddispatch_get_itable_index(st);
					}

					add_itable_to_itt(initializer, st, itable, itt_index, n_itable_count);
				}
			}

			iterator = iterator->next;
		}
	}

	free_klass_iterator(iterator);
}


void ddispatch_setup_vtable(ir_type *klass)
{
	assert(is_Class_type(klass));

	ir_entity *vtable = oo_get_class_vtable_entity(klass);
	if (vtable == NULL)
		return;
	if (get_entity_initializer(vtable) != NULL)
		return;

	unsigned vtable_size;
	ir_type *superclass = oo_get_class_superclass(klass);
	if (superclass) {
		vtable_size = oo_get_class_vtable_size(superclass);
	} else {
		vtable_size = ddispatch_model.index_of_first_method-ddispatch_model.vptr_points_to_index;
	}

	// assign vtable ids
	for (size_t i = 0; i < get_class_n_members(klass); i++) {
		ir_entity *member = get_class_member(klass, i);
		if (! is_method_entity(member))
			continue;
		if (oo_get_method_exclude_from_vtable(member))
			continue;

		int vtable_id;
		if (oo_get_method_is_inherited(member)) {
			// vtable_id is already assigned (setup_vtable of the superclass
			// did that)
			ir_entity *super
				= oo_get_entity_overwritten_superclass_entity(member);
			vtable_id = oo_get_method_vtable_index(super);
			assert(vtable_id != -1);
		} else {
			// assign new vtable id
			vtable_id = vtable_size++;
		}
		oo_set_method_vtable_index(member, vtable_id);
	}
	oo_set_class_vtable_size(klass, vtable_size);

	// the vtable currently is an array of pointers
	unsigned type_reference_size = get_type_size_bytes(type_reference);
	ir_type *vtable_type = new_type_array(type_reference);
	size_t vtable_ent_size = vtable_size + ddispatch_model.vptr_points_to_index;
	set_array_size_int(vtable_type, vtable_ent_size);
	set_type_size_bytes(vtable_type, type_reference_size * vtable_ent_size);
	set_type_state(vtable_type, layout_fixed);
	set_entity_type(vtable, vtable_type);
	set_entity_alignment(vtable, 32);

	ir_graph *const_code = get_const_code_irg();
	ir_initializer_t * init = create_initializer_compound(vtable_ent_size);

	if (superclass != NULL) {
		unsigned   superclass_vtable_size   = oo_get_class_vtable_size(superclass);
		ir_entity *superclass_vtable_entity = oo_get_class_vtable_entity(superclass);
		assert (superclass_vtable_entity);
		ir_initializer_t *superclass_vtable_init = get_entity_initializer(superclass_vtable_entity);

		// copy vtable initialization from superclass
		for (unsigned i = ddispatch_model.vptr_points_to_index;
			          i < superclass_vtable_size+ddispatch_model.vptr_points_to_index;
			          i++) {
				ir_initializer_t *superclass_vtable_init_value = get_initializer_compound_value(superclass_vtable_init, i);
				set_initializer_compound_value (init, i, superclass_vtable_init_value);
		}
	}

	// setup / replace vtable entries to point to clazz's implementation
	for (size_t i = 0; i < get_class_n_members(klass); i++) {
		ir_entity *member = get_class_member(klass, i);
		if (is_method_entity(member)) {
			int member_vtid = oo_get_method_vtable_index(member);
			if (member_vtid != -1) {
				ir_node *symconst_node = new_r_Address(const_code, member);
				ir_initializer_t *val = create_initializer_const(symconst_node);
				set_initializer_compound_value (init, member_vtid+ddispatch_model.vptr_points_to_index, val);
			}
		}
	}

	(*ddispatch_model.init_vtable_slots)(klass, init, vtable_ent_size);

	set_entity_initializer(vtable, init);
}

void ddispatch_lower_Call(ir_node* call)
{
	assert(is_Call(call));

	ir_node *callee = get_Call_ptr(call);
	if (!is_Proj(callee))
		return;
	ir_node *methodsel = get_Proj_pred(callee);
	if (!is_MethodSel(methodsel))
		return;

	ir_node   *objptr = get_MethodSel_ptr(methodsel);
	ir_node   *mem    = get_MethodSel_mem(methodsel);
	ir_entity *method = get_MethodSel_entity(methodsel);
	assert(is_method_entity(method));

	ir_type *classtype = get_entity_owner(method);
	assert(is_Class_type(classtype));

	ddispatch_binding binding = oo_get_entity_binding(method);
	if (binding == bind_unknown)
		panic("method %s has no binding specified", get_entity_name(method));

	/* If the call has been explicitly marked as statically bound, then obey. */
	if (oo_get_call_is_statically_bound(call))
		binding = bind_static;
	if (binding == bind_dynamic && oo_get_method_is_final(method))
		binding = bind_static;
	if (binding == bind_dynamic && oo_get_class_is_final(classtype))
		binding = bind_static;

	ir_graph *irg   = get_irn_irg(call);
	ir_node  *block = get_nodes_block(call);

	ir_node *new_mem = mem;
	ir_node *new_res;
	switch (binding) {
	case bind_static:
		new_res = new_r_Address(irg, method);
		break;

	case bind_dynamic: {
		ir_entity *vptr_entity  = oo_get_class_vptr_entity(classtype);
		ir_type   *vptr_type    = get_entity_type(vptr_entity);
		ir_node   *vptr         = new_r_Member(block, objptr, vptr_entity);

		ir_node   *vtable_load  = new_r_Load(block, mem, vptr, mode_reference, vptr_type, cons_none);
		ir_node   *vtable_addr  = new_r_Proj(vtable_load, mode_reference, pn_Load_res);
		ir_node   *vtable_mem   = new_r_Proj(vtable_load, mode_M, pn_Load_M);

		int        vtable_id    = oo_get_method_vtable_index(method);
		assert(vtable_id != -1);

		unsigned type_ref_size  = get_type_size_bytes(type_reference);
		ir_mode *mode_offset    = get_reference_offset_mode(mode_reference);
		ir_node *vtable_offset  = new_r_Const_long(irg, mode_offset, vtable_id * type_ref_size);
		ir_node *funcptr_addr   = new_r_Add(block, vtable_addr, vtable_offset, mode_reference);
		ir_node *callee_load    = new_r_Load(block, vtable_mem, funcptr_addr, mode_reference, vptr_type, cons_none);
		new_res                 = new_r_Proj(callee_load, mode_reference, pn_Load_res);
		new_mem                 = new_r_Proj(callee_load, mode_M, pn_Load_M);
		break;
	}
	case bind_interface:
		new_res = (*ddispatch_model.construct_interface_lookup)(objptr, classtype, method, irg, block, &new_mem);
		break;

	default:
		panic("Cannot lower call.");
	}

	ir_node *in[] = {
		[pn_MethodSel_M]   = new_mem,
		[pn_MethodSel_res] = new_res,
	};
	turn_into_tuple(methodsel, ARRAY_SIZE(in), in);
}

void ddispatch_prepare_new_instance(dbg_info *dbgi, ir_node *block, ir_node *objptr, ir_node **mem, ir_type* klass)
{
	ir_graph *irg = get_irn_irg(block);
	assert(is_Class_type(klass));

	ir_node   *cur_mem         = *mem;
	ir_entity *vptr_entity     = oo_get_class_vptr_entity(klass);
	ir_type   *vptr_type       = get_entity_type(vptr_entity);
	ir_node   *vptr            = new_rd_Member(dbgi, block, objptr, vptr_entity);

	ir_node   *vptr_target     = NULL;
	ir_entity *vtable_entity   = oo_get_class_vtable_entity(klass);
	if (vtable_entity) {
		ir_node   *vtable_symconst = new_r_Address(irg, vtable_entity);
		ir_mode   *mode_offset     = get_reference_offset_mode(mode_reference);
		long       offset          = ddispatch_model.vptr_points_to_index * get_type_size_bytes(type_reference);
		ir_node   *const_offset    = new_r_Const_long(irg, mode_offset, offset);
		vptr_target                = new_rd_Add(dbgi, block, vtable_symconst, const_offset, mode_reference);
	} else {
		vptr_target                = new_r_Const_long(irg, mode_P, 0);
	}

	ir_node   *vptr_store      = new_rd_Store(dbgi, block, cur_mem, vptr, vptr_target, vptr_type, cons_floats);
	cur_mem                    = new_r_Proj(vptr_store, mode_M, pn_Store_M);

	*mem = cur_mem;
}

void ddispatch_set_vtable_layout(unsigned vptr_points_to_index, unsigned index_of_first_method, unsigned index_of_rtti_ptr, init_vtable_slots_t func)
{
	assert (index_of_first_method >= vptr_points_to_index);
	assert (func);

	ddispatch_model.vptr_points_to_index  = vptr_points_to_index;
	ddispatch_model.index_of_first_method = index_of_first_method;
	ddispatch_model.index_of_rtti_ptr     = index_of_rtti_ptr;
	ddispatch_model.init_vtable_slots     = func;
}

void ddispatch_set_interface_lookup_constructor(construct_interface_lookup_t func)
{
	assert (func);
	ddispatch_model.construct_interface_lookup = func;
}

void ddispatch_set_abstract_method_ident(ident* ami)
{
	assert (ami);
	ddispatch_model.abstract_method_ident = ami;
}

unsigned ddispatch_get_vptr_points_to_index(void)
{
	return ddispatch_model.vptr_points_to_index;
}

unsigned ddispatch_get_index_of_rtti_ptr(void)
{
	return ddispatch_model.index_of_rtti_ptr;
}

unsigned ddispatch_get_index_of_first_method(void)
{
	return ddispatch_model.index_of_first_method;
}
