#ifndef OO_DDISPATCH_H
#define OO_DDISPATCH_H

/*
 * Layout of the created dynamic dispatch table (vtable):
 *
 * Default layout:
 *
 * _ZTVNxyzE:
 *   <vtable slot 0> runtime classinfo
 *   <vtable slot 1> addr(first method)
 *   ...
 *   <vtable slot n> addr(last method)
 *
 * More complicated layouts are possible, use ddispatch_set_vtable_layout(..).
 * The resulting vtable will look like this:
 *
 * _ZTVNxyzE:
 *   0               uninitialized*
 *   0               uninitialized*
 *   <vtable slot 0> runtime classinfo  <-- vtable_vptr_points_to_index (e.g., == 2)
 *   <vtable slot 1> uninitialized*
 *   <vtable slot 2> addr(first method) <-- vtable_index_of_first_method (e.g., == 4)
 *   ...
 *   <vtable slot n> addr(last method)
 *
 *   *) use callback function vtable_init_slots to set ir_initializer_t's
 *      for the uninitialized slots.
 */

#include <libfirm/firm.h>

/*
 * This property specifies how a
 *
 * class   method entity
 *     \   /
 *      Sel
 *       |
 *      Call
 *
 * construction should be lowered.
 *
 * bind_static   : no further indirection should be performed, just call the specified method.
 * bind_dynamic  : use the vtable mechanism to determine the real callee.
 * bind_interface: invoke a special lookup method at runtime to get the callee.
 *
 * NOTE: This does not distinguish class/static vs. instance methods or fields.
 *
 */
typedef enum {
	bind_unknown,
	bind_static,
	bind_dynamic,
	bind_interface
} ddispatch_binding;

typedef enum {
	call_runtime_lookup = 0,
	call_searched_itable = 1,
	call_itable_indexed = 2,
	call_move2front = 4
} ddispatch_interface_call;


typedef void     (*init_vtable_slots_t)           (ir_type* klass, ir_initializer_t *vtable_init, unsigned vtable_size);
typedef ir_node* (*construct_interface_lookup_t)  (ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem);

void ddispatch_init(void);
void ddispatch_deinit(void);
void ddispatch_setup_vtable(ir_type *klass);
void ddispatch_lower_Call(ir_node* call);
void ddispatch_prepare_new_instance(dbg_info *dbgi, ir_node *block, ir_node *objptr, ir_node **mem, ir_type* klass);

void ddispatch_setup_itable(ir_type *klass);
int ddispatch_get_itable_method_index(ir_type *interface, ir_entity *method);
ir_entity *ddispatch_get_itable_id(ir_type *interface);
unsigned ddispatch_get_itable_index(ir_type *interface);
ir_type *ddispatch_get_itt_entry_type(void);


void ddispatch_set_vtable_layout(unsigned vptr_points_to_index, unsigned index_of_first_method, unsigned index_of_rtti_ptr, init_vtable_slots_t func);
void ddispatch_set_interface_lookup_constructor(construct_interface_lookup_t func);
void ddispatch_set_abstract_method_entity(ir_entity *method);

unsigned ddispatch_get_vptr_points_to_index(void);
unsigned ddispatch_get_index_of_rtti_ptr(void);
unsigned ddispatch_get_index_of_first_method(void);

#endif
