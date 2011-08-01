#ifndef DEBUGINFO_H
#define DEBUGINFO_H

#include <libfirm/firm.h>

void debuginfo_init(void);
const dbg_info *create_debuginfo(ident *filename, unsigned linenr,
                                 unsigned column, unsigned length);
void debuginfo_deinit(void);

#endif

