#include <stdbool.h>
#include "liboo/rts_types.h"
#include "../adt/error.h"
#include "rt.h"
#include "types.h"

#define ITT_MOVE2FRONT_AREA 5

void *oo_rt_lookup_interface_method(const class_info_t *klass,
                                    const string_const_t *method_name)
{
	const class_info_t *k = klass;
	do {
		for (uint32_t i = 0; i < k->n_methods; i++) {
			const method_info_t *method = &k->methods[i];
			if (string_const_equals(method->name, method_name)) {
				return method->funcptr;
			}
		}
		// not found, try the superclass
		k = k->superclass;
	} while (k != NULL);

	panic("Interface lookup for %s in %s failed", get_string_const_chars(method_name), get_string_const_chars(klass->name));
}

void *oo_searched_itable_method_m2f(const object_t *obj, void *interface_id, int32_t offset)
{
	itt_entry_t *itt = obj->vptr->itt;

	int32_t i = itt[0].next;

	int32_t counter = 1;
	do {
		if (itt[i].id == interface_id) {
			void *method = itt[i].itable[offset];

			if (counter > ITT_MOVE2FRONT_AREA) {
				if (itt[i].prev > 0)
					itt[itt[i].prev].next = itt[i].next;
				if (itt[i].next > 0)
					itt[itt[i].next].prev = itt[i].prev;

				itt[itt[0].next].prev = i;
				itt[i].next = itt[0].next;

				itt[0].next = i;
				itt[i].prev = 0;
			}

			return method;
		}
		counter++;
	}
	while ((i = itt[i].next) > 0);

	panic("itable not found");
}

void *oo_searched_itable_method(const object_t *obj, void *interface_id, int32_t offset)
{
	itt_entry_t *itt = obj->vptr->itt;

	int32_t i = 1;

	int32_t counter = 1;
	do {
		if (itt[i].id == interface_id) {
			return itt[i].itable[offset];
		}
		counter++;
	}
	while (itt[++i].id != NULL);

	panic("itable not found");
}
