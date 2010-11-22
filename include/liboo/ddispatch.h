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

typedef int  (*vtable_create_pred_t)      (ir_type* classtype);
typedef int  (*vtable_include_pred_t)     (ir_entity* method);
typedef int  (*vtable_is_abstract_pred_t) (ir_entity* method);
typedef void (*vtable_init_slots_t)       (ir_type* classtype, ir_initializer_t *vtable_init, unsigned vtable_size);

typedef struct {
	vtable_create_pred_t      *vtable_create_pred;
	vtable_include_pred_t     *vtable_include_pred;
	vtable_is_abstract_pred_t *vtable_is_abstract_pred;
	vtable_init_slots_t       *vtable_init_slots;

	ident                     *vtable_abstract_method_ident;

	unsigned                   vtable_vptr_points_to_index;
	unsigned                   vtable_index_of_first_method;
} ddispatch_params;

void ddispatch_init(ddispatch_params params);
void ddispatch_setup_vtable(ir_type *klass);


#endif
