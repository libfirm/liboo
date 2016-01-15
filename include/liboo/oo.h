#ifndef OO_OO_H
#define OO_OO_H

#include <libfirm/firm.h>
#include <stdbool.h>

#include "ddispatch.h"
#include "rta.h"

void oo_init(void);
void oo_deinit(void);
void oo_lower(void);

void oo_set_interface_call_type(ddispatch_interface_call type);
ddispatch_interface_call oo_get_interface_call_type(void);


unsigned oo_get_class_uid(ir_type *classtype);
void oo_set_class_uid(ir_type *classtype, unsigned uid);
ir_entity *oo_get_class_vtable_entity(ir_type *classtype);
void oo_set_class_vtable_entity(ir_type *classtype, ir_entity *vtable);
unsigned oo_get_class_vtable_size(ir_type *classtype);
void oo_set_class_vtable_size(ir_type *classtype, unsigned vtable_size);
ir_entity *oo_get_class_vptr_entity(ir_type *classtype);
void oo_set_class_vptr_entity(ir_type *classtype, ir_entity *vptr);
ir_entity *oo_get_class_rtti_entity(ir_type *classtype);
void oo_set_class_rtti_entity(ir_type *classtype, ir_entity *rtti);
ir_entity *oo_get_class_itt_entity(ir_type *classtype);
void oo_set_class_itt_entity(ir_type *classtype, ir_entity *itt);
bool oo_get_class_is_interface(ir_type *classtype);
void oo_set_class_is_interface(ir_type *classtype, bool is_interface);
bool oo_get_class_is_abstract(ir_type *classtype);
void oo_set_class_is_abstract(ir_type *classtype, bool is_abstract);
bool oo_get_class_is_final(ir_type *classtype);
void oo_set_class_is_final(ir_type *classtype, bool is_final);
bool oo_get_class_is_extern(ir_type *classtype);
void oo_set_class_is_extern(ir_type *classtype, bool is_extern);

void *oo_get_type_link(ir_type *type);
void oo_set_type_link(ir_type *type, void* link);

bool oo_get_method_exclude_from_vtable(ir_entity *method);
void oo_set_method_exclude_from_vtable(ir_entity *method, bool exclude_from_vtable);
int  oo_get_method_vtable_index(ir_entity *method);
void oo_set_method_vtable_index(ir_entity *method, int vtable_slot);
bool oo_get_method_is_abstract(ir_entity *method);
void oo_set_method_is_abstract(ir_entity *method, bool is_abstract);
bool oo_get_method_is_final(ir_entity *method);
void oo_set_method_is_final(ir_entity *method, bool is_final);
bool oo_get_method_is_inherited(ir_entity *method);
void oo_set_method_is_inherited(ir_entity *method, bool is_inherited);

bool oo_get_field_is_transient(ir_entity *field);
void oo_set_field_is_transient(ir_entity *field, bool is_transient);

ddispatch_binding oo_get_entity_binding(ir_entity *entity);
void oo_set_entity_binding(ir_entity *entity, ddispatch_binding binding);
void *oo_get_entity_link(ir_entity *entity);
void oo_set_entity_link(ir_entity *entity, void* link);

void oo_set_call_is_statically_bound(ir_node *call, bool bind_statically);
bool oo_get_call_is_statically_bound(ir_node *call);

ir_type *oo_get_class_superclass(ir_type *klass);
ir_entity *oo_get_entity_overwritten_superclass_entity(ir_entity *entity);

void oo_copy_entity_info(ir_entity *src, ir_entity *dest);

#endif
