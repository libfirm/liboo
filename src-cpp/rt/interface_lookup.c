#include <stdbool.h>
#include "liboo/rts_types.h"
#include "../adt/error.h"
#include "rt.h"

void *oo_rt_lookup_interface_method(const class_info_t *klass, const string_const_t *method_name)
{
	for (uint32_t i = 0; i < klass->n_methods; i++) {
		if (string_const_equals(klass->methods[i].name, method_name)) {
			return klass->methods[i].funcptr;
		}
	}

	// method is not declared in the current class, try the superclass
	if (klass->superclass)
		return oo_rt_lookup_interface_method(klass->superclass, method_name);

	panic("Interface lookup for %s in %s failed", get_string_const_chars(method_name), get_string_const_chars(klass->name));
}
