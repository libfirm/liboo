#include "../adt/error.h"

extern void oo_rt_abstract_method_error(void);

void oo_rt_abstract_method_error(void)
{
	panic("Cannot invoke abstract method.");
}
