#ifndef OO_DDISPATCH_H
#define OO_DDISPATCH_H

/*
 * Layout of the created dynamic dispatch table (vtable):
 *
 * Default layout:
 *
 * _ZTVNxyzE:
 *   <vtable slot 0> addr(first method)
 *   ...
 *   <vtable slot n> addr(last method)
 *
 * More complicated layouts are possible, use ddispatch_set_vtable_layout(..).
 * The resulting vtable will look like this:
 *
 * _ZTVNxyzE:
 *   0               uninitialized*
 *   0               uninitialized*
 *   <vtable slot 0> uninitialized*     <-- vtable_vptr_points_to_index (e.g., == 2)
 *   <vtable slot 1> uninitialized*
 *   <vtable slot 2> addr(first method) <-- vtable_index_of_first_method (e.g., == 4)
 *   ...
 *   <vtable slot n> addr(last method)
 *
 *   *) use callback function vtable_init_slots to set ir_initializer_t's
 *      for the uninitialized slots.
 */

#include <libfirm/firm.h>

typedef enum { bind_unknown, bind_static, bind_dynamic, bind_interface } ddispatch_binding;

typedef void     (*init_vtable_slots_t)           (ir_type* klass, ir_initializer_t *vtable_init, unsigned vtable_size);
typedef ir_node* (*construct_interface_lookup_t)  (ir_node *objptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem);

void ddispatch_init(void);
void ddispatch_setup_vtable(ir_type *klass);
void ddispatch_lower_Call(ir_node* call);
void ddispatch_prepare_new_instance(ir_type* klass, ir_node *objptr, ir_graph *irg, ir_node *block, ir_node **mem);

void ddispatch_set_vtable_layout(unsigned vptr_points_to_index, unsigned index_of_first_method, init_vtable_slots_t func);
void ddispatch_set_interface_lookup_constructor(construct_interface_lookup_t func);
void ddispatch_set_abstract_method_ident(ident* ami);

#endif
