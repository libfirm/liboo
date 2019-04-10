
/*
 * Liboo garbage collector.
 */

/**
 * @file	oo_gc.h
 * @brief	Liboo garbage collector which is able to remove methods before oo lowering.
 * 		This gc does need the whole program to work correctly
 * @author	Daniel Biester
 * @date	2019
 */




#ifndef OO_GC_H
#define OO_GC_H


#include <stdbool.h>
#include <libfirm/firm.h>
#include "../../src-cpp/adt/cpmap.h"
#include "../../src-cpp/adt/cpset.h"


/**
 * Garbage collect entities before oo_lowering
 * @param entry_points the entry points to the gc
 */
void oo_garbage_collect_entities(ir_entity **entry_points);

#endif
