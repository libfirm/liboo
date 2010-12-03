#ifndef OO_H
#define OO_H

/*
 * Aggregate header file for liboo.
 */

#include <liboo/ddispatch.h>
#include <libfirm/firm.h>

#include <stdbool.h>

void oo_init(void);
void oo_deinit(void);
void lower_oo(void);

bool  get_class_needs_vtable(ir_type *classtype);
void  set_class_needs_vtable(ir_type *classtype, bool needs_vtable);
ir_entity *get_class_vptr_entity(ir_type *classtype);
void  set_class_vptr_entity(ir_type *classtype, ir_entity *vptr);
void *get_oo_type_link(ir_type *type);
void  set_oo_type_link(ir_type *type, void* link);
bool  get_method_include_in_vtable(ir_entity *method);
void  set_method_include_in_vtable(ir_entity *method, bool include_in_vtable);
bool  get_method_is_abstract(ir_entity *method);
void  set_method_is_abstract(ir_entity *method, bool is_abstract);
ddispatch_binding get_entity_binding(ir_entity *method);
void  set_entity_binding(ir_entity *method, ddispatch_binding binding);
void *get_oo_entity_link(ir_entity *entity);
void  set_oo_entity_link(ir_entity *entity, void* link);

#endif
