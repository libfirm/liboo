#ifndef OO_OO_H
#define OO_OO_H

/*
 * Aggregate header file for liboo.
 */

#include <liboo/ddispatch.h>
#include <libfirm/firm.h>

#include <stdbool.h>

void oo_init(void);
void oo_deinit(void);
void oo_lower(void);

bool  get_class_needs_vtable(ir_type *classtype);
void  set_class_needs_vtable(ir_type *classtype, bool needs_vtable);
ir_entity *oo_get_class_vptr_entity(ir_type *classtype);
void  oo_set_class_vptr_entity(ir_type *classtype, ir_entity *vptr);
void *oo_get_type_link(ir_type *type);
void  oo_set_type_link(ir_type *type, void* link);
bool  get_method_include_in_vtable(ir_entity *method);
void  set_method_include_in_vtable(ir_entity *method, bool include_in_vtable);
bool  oo_get_method_is_abstract(ir_entity *method);
void  oo_set_method_is_abstract(ir_entity *method, bool is_abstract);
ddispatch_binding oo_get_entity_binding(ir_entity *entity);
void  oo_set_entity_binding(ir_entity *entity, ddispatch_binding binding);
void *oo_get_entity_link(ir_entity *entity);
void  oo_set_entity_link(ir_entity *entity, void* link);

#endif
