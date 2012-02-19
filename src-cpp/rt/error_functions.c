#include "../adt/error.h"
#include "rt.h"

void oo_rt_abstract_method_error(void)
{
	panic("Cannot invoke abstract method.");
}
