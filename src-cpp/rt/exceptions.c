#ifdef LIBOO_USE_EXCEPTIONS
#define UNW_LOCAL_ONLY
/* Workaround for buggy glibcs. */
#define _BSD_SOURCE
#include <libunwind.h>
#undef _BSD_SOURCE
#endif

#include "liboo/rts_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

extern void firm_personality(void *exception_object);

#ifdef LIBOO_USE_EXCEPTIONS
extern __thread void *__oo_rt_exception_object__;

static void oo_rt_unwind(void)
{
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
			for (unsigned i = 0; i < lsda->n_entries; i++) {
				if (ip == (unsigned)lsda->entries[i].ip) {
					unw_set_reg(&cursor, UNW_REG_IP, (unw_word_t)lsda->entries[i].handler);
					unw_resume(&cursor);
				}
			}
		}
	}
}
#endif

__attribute__ ((unused))
void firm_personality(void *exception_object)
{
#ifdef LIBOO_USE_EXCEPTIONS
	__oo_rt_exception_object__ = exception_object;
	oo_rt_unwind();
#endif

	fprintf(stderr, "UNCAUGHT EXCEPTION 0x%p\n", exception_object);
	exit(-1);
}
