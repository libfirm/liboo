#ifndef OO_H
#define OO_H

/*
 * Aggregate header file for liboo.
 */

#include <liboo/ddispatch.h>
#include <liboo/dmemory.h>
#include <liboo/mangle.h>

void oo_init(void);
void oo_deinit(void);
void lower_oo(void);

#endif
