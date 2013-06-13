#include <stdbool.h>
#include "liboo/rts_types.h"
#include "../adt/error.h"
#include "rt.h"

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
