#ifndef DDISPATCH_H
#define DDISPATCH_H

/*
 * Layout of the created dynamic dispatch table (vtable):
 *
 * _ZTVNxyzE:
 *   0               uninitialized*
 *   0               uninitialized*
 *   <vtable slot 0> uninitialized*     <-- params.vtable_vptr_points_to_index (e.g., == 2)
 *   <vtable slot 1> uninitialized*
 *   <vtable slot 2> addr(first method) <-- params.vtable_index_of_first_method (e.g., == 4)
 *   ...
 *   <vtable slot n> addr(last method)
 *
 *   *) use callback function params.vtable_init_slots to set ir_initializer_t's
 *      for the uninitialized slots.
 */

#include <libfirm/firm.h>

typedef int  (*vtable_create_pred_t)      (ir_type* klass);
typedef int  (*vtable_include_pred_t)     (ir_entity* method);
typedef int  (*vtable_is_abstract_pred_t) (ir_entity* method);
typedef void (*vtable_init_slots_t)       (ir_type* klass, ir_initializer_t *vtable_init, unsigned vtable_size);

typedef enum { bind_static, bind_dynamic, bind_interface, bind_builtin, bind_already_bound } ddispatch_binding;
typedef ddispatch_binding (*call_decide_binding_t) (ir_node* call);
typedef ir_node* (*call_lookup_interface_method_t) (ir_node *obptr, ir_type *iface, ir_entity *method, ir_graph *irg, ir_node *block, ir_node **mem);
typedef void (*call_lower_builtin_t)        (ir_node* call);

typedef struct {
	vtable_create_pred_t       vtable_create_pred;
	vtable_include_pred_t      vtable_include_pred;
	vtable_is_abstract_pred_t  vtable_is_abstract_pred;
	vtable_init_slots_t        vtable_init_slots;
	ident                     *vtable_abstract_method_ident;
	unsigned                   vtable_vptr_points_to_index;
	unsigned                   vtable_index_of_first_method;

	call_decide_binding_t      call_decide_binding;
	call_lookup_interface_method_t call_lookup_interface_method;
	call_lower_builtin_t       call_lower_builtin;
	ir_entity                **call_vptr_entity;
} ddispatch_params;

void ddispatch_init(ddispatch_params params);
void ddispatch_setup_vtable(ir_type *klass);
void ddispatch_lower_Call(ir_node* call);
void ddispatch_prepare_new_instance(ir_type* klass, ir_node *objptr, ir_graph *irg, ir_node *block, ir_node **mem);

#endif
