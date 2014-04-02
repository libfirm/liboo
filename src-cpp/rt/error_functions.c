#include "../adt/error.h"
#include "rt.h"

void oo_rt_abstract_method_error(void)
{
	panic("Cannot invoke abstract method.");
}

void oo_rt_method_optimized_away_error(void)
{
	panic("Cannot invoke method. It was optimized away.");
}
