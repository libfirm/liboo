
#include "liboo/rts_types.h"
#include "../adt/error.h"
#include <stdbool.h>

extern bool oo_rt_instanceof(class_info_t *objclass, class_info_t *refclass);

__attribute__ ((unused))
bool oo_rt_instanceof(class_info_t *objclass, class_info_t *refclass)
{
	if (objclass == refclass)
		return true;

	if (objclass->superclass && oo_rt_instanceof(objclass->superclass, refclass))
		return true;

	if (objclass->n_interfaces > 0) {
		for (int i = 0; i < objclass->n_interfaces; i++) {
			class_info_t *ci = objclass->interfaces[i];
			if (oo_rt_instanceof(ci, refclass))
				return true;
		}
	}

	return false;
}
