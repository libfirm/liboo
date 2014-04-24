#include <stdlib.h>
#include <stdint.h>

#include <libfirm/firm.h>
#include "liboo/debuginfo.h"
#include "adt/obst.h"
#include "adt/error.h"
#include "adt/xmalloc.h"

typedef struct debuginfo_t {
	ident *filename;
	uint32_t line;
	uint16_t column;
	uint16_t length;
} debuginfo_t;

static struct obstack obst;

static src_loc_t dbg_retrieve(const dbg_info *dbg)
{
	const debuginfo_t *info = (const debuginfo_t*) dbg;
	src_loc_t loc = { NULL, 0, 0 };
	if (info == NULL)
		return loc;
	loc.file   = get_id_str(info->filename);
	loc.line   = info->line;
	loc.column = info->column;
	return loc;
}

void debuginfo_init(void)
{
	obstack_init(&obst);
	ir_set_debug_retrieve(dbg_retrieve);
}

void debuginfo_deinit(void)
{
	obstack_free(&obst, NULL);
}

const dbg_info *create_debuginfo(ident *filename, unsigned line,
                                 unsigned column, unsigned length)
{
	debuginfo_t *info = OALLOC(&obst, debuginfo_t);
	info->filename = filename;
	info->line     = (uint32_t) line;
	info->column   = (uint16_t) column;
	info->length   = (uint16_t) length;
	return (const dbg_info*) info;
}
