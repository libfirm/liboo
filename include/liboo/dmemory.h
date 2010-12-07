#ifndef OO_DMEMORY_H
#define OO_DMEMORY_H

#include <libfirm/firm.h>

typedef ir_node* (*alloc_object_t)    (ir_type *type, ir_graph *irg, ir_node *block, ir_node **mem);
typedef ir_node* (*alloc_array_t)     (ir_type *eltype, ir_node *count, ir_graph *irg, ir_node *block, ir_node **mem);
typedef ir_node* (*get_arraylength_t) (ir_node *objptr, ir_graph *irg, ir_node *block, ir_node **mem);

void       dmemory_init(void);
void       dmemory_lower_Alloc(ir_node* alloc);
void       dmemory_lower_arraylength(ir_node* call);
ir_entity* dmemory_get_arraylength_entity(void);
void       dmemory_set_allocation_methods(alloc_object_t ao_func, alloc_array_t aa_func, get_arraylength_t ga_func);

#endif
