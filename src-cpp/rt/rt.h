#ifndef LIBOO_RT_H
#define LIBOO_RT_H

#include <stdbool.h>
#include "liboo/rts_types.h"

/* forward declarations for runtime interface. (Though most users probably
 * won't use this from a C-Compiler... */

void oo_rt_abstract_method_error(void);
bool oo_rt_instanceof(class_info_t *objclass, class_info_t *refclass);
void *oo_rt_lookup_interface_method(class_info_t *klass, string_const_t *method_name);

#endif
