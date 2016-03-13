#ifdef LIBOO_EXCEPTION_SUPPORT

/* Workaround for buggy glibcs. */
#define _DEFAULT_SOURCE
#include <stdio.h>
#undef _DEFAULT_SOURCE
#include <stdlib.h>
#include <assert.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "liboo/rts_types.h"

extern void firm_personality(void *exception_object);

extern void *__oo_rt_exception_object__;

__attribute__ ((unused))
void firm_personality(void *exception_object)
{
	__oo_rt_exception_object__ = exception_object;

	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip;
	unw_proc_info_t pi;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	while (unw_step(&cursor) > 0) {
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_proc_info(&cursor, &pi);

		if (pi.lsda != 0 && (void (*)(void*))pi.handler == firm_personality) {
			lsda_t *lsda = (lsda_t*) pi.lsda;
			for (uint64_t i = 0; i < lsda->n_entries; i++) {
				if (ip == lsda->entries[i].ip) {
					unw_set_reg(&cursor, UNW_REG_IP, (unw_word_t)lsda->entries[i].handler);
					unw_resume(&cursor);
				}
			}
		}
	}

	fprintf(stderr, "UNCAUGHT EXCEPTION %p\n", exception_object);
	abort();
}

#else
extern void liboo_dummy_func(void);
void liboo_dummy_func(void)
{
}
#endif
