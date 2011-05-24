
#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "liboo/rts_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

extern void oo_rt_throw(void *exception_object);

extern __thread void *__oo_rt_exception_object__;

static void show_backtrace (void) {
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip;
	unw_proc_info_t pi;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	while (unw_step(&cursor) > 0) {
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_proc_info(&cursor, &pi);

		if (pi.lsda != 0 && (void*)pi.handler == oo_rt_throw) {
			lsda_t *lsda = (lsda_t*) pi.lsda;
			for (unsigned i = 0; i < lsda->n_entries; i++) {
				if (ip == (unsigned)lsda->entries[i].ip+5) {
					fprintf(stderr, "[UNWIND] resuming...\n");
					unw_set_reg(&cursor, UNW_REG_IP, (unw_word_t)lsda->entries[i].handler);
					unw_resume(&cursor);
				}
			}
		}
	}
}

__attribute__ ((unused))
void oo_rt_throw(void *exception_object)
{
	__oo_rt_exception_object__ = exception_object;

	show_backtrace();

	exit(-1);
}
