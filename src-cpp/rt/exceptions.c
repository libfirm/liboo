
#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "liboo/rts_types.h"
#include <stdlib.h>
#include <stdio.h>

extern void oo_rt_throw(void *exception_object);

extern __thread void *__oo_rt_exception_object__;

static void show_backtrace (void) {
  unw_cursor_t cursor; unw_context_t uc;
  unw_word_t ip, sp;
  unw_proc_info_t pi;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);

  while (unw_step(&cursor) > 0) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    unw_get_proc_info(&cursor, &pi);

    char buffer[1024];
    unw_word_t dummy;
    unw_get_proc_name(&cursor, buffer, 1024, &dummy);
    printf ("ip = %lx, sp = %lx\t%x\t%x\t%s\n", (long) ip, (long) sp, (unsigned)pi.lsda, (unsigned)pi.handler, buffer);

    if (pi.lsda != 0 && (void*)pi.handler == oo_rt_throw) {
    	lsda_t *lsda = (lsda_t*) pi.lsda;
    	printf("LSDA: %d\n", lsda->n_entries);
    	for (unsigned i = 0; i < lsda->n_entries; i++) {
    		printf("  %x -> %x\n", (unsigned)lsda->entries[i].ip+5, lsda->entries[i].handler);
    		if (ip == (unsigned)lsda->entries[i].ip+5) {
    			printf("resuming...\n");
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
	fprintf(stderr, "Caught exception 0x%x\n", (unsigned) exception_object);
	class_info_t *ct = get_class_info(exception_object);
	fprintf(stderr, "Exception class: %s\n", get_string_const_chars(ct->name));
	__oo_rt_exception_object__ = exception_object;

	show_backtrace();

	exit(-1);
}
