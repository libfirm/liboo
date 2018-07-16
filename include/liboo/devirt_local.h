#ifndef OO_LOCAL_DEVIRT_H
#define OO_LOCAL_DEVIRT_H

#include <libfirm/firm.h>

void devirt_set_detection_callbacks(ir_entity *(*detect_call)(ir_node *call));

int devirtualize_calls_to_local_objects(ir_entity **entry_points);

#endif
