#ifndef DMEMORY_H
#define DMEMORY_H

#include <libfirm/firm.h>

typedef ir_node* (*heap_alloc_object_t) (ir_type *type, ir_graph *irg, ir_node *block, ir_node **mem);
typedef ir_node* (*heap_alloc_array_t)  (ir_type *eltype, ir_node *count, ir_graph *irg, ir_node *block, ir_node **mem);
typedef ir_node* (*arraylength_get_t)   (ir_node *objptr, ir_graph *irg, ir_node *block, ir_node **mem);

typedef struct {
	heap_alloc_object_t heap_alloc_object;
	heap_alloc_array_t  heap_alloc_array;
	arraylength_get_t   arraylength_get;
} dmemory_params;

void dmemory_init(dmemory_params params);
void dmemory_lower_Alloc(ir_node* alloc);
void dmemory_lower_arraylength(ir_node* call);

#endif
