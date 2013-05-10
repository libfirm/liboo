#ifndef OO_DMEMORY_H
#define OO_DMEMORY_H

#include <libfirm/firm.h>

typedef ir_node* (*get_arraylength_t) (dbg_info *dbgi, ir_node *block, ir_node *objptr, ir_node **mem);

ir_node *dmemory_default_get_arraylength(dbg_info *dbgi, ir_node *block, ir_node *objptr, ir_node **mem);

void dmemory_init(void);
void dmemory_lower_Arraylength(ir_node* arraylength);
void dmemory_set_allocation_methods(get_arraylength_t ga_func);

#endif
