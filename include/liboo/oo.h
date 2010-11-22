#ifndef OO_H
#define OO_H

/*
 * Aggregate header file for liboo.
 *
 * Initialization should be done through oo_init(..).
 * Name mangling can be configured further through
 * functions defined in mangle.h.
 *
 * Then, simply call lower_oo().
 */

#include <liboo/ddispatch.h>
#include <liboo/dmemory.h>
#include <liboo/mangle.h>

void oo_init(ddispatch_params ddparams, dmemory_params dmparams);
void oo_deinit(void);
void lower_oo(void);

#endif
