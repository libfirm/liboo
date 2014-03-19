/*
 * This file is part of liboo.
 */

/**
 * @file	rta.h
 * @brief	Devirtualization of dynamically linked calls through Rapid Type Analysis
 * @author	Steffen Knoth
 * @date	2014
 */


#ifndef OO_RTA_H
#define OO_RTA_H


#include <libfirm/firm.h>


/** runs Rapid Type Analysis and then tries to devirtualize dynamically linked calls and to discard unneeded classes and methods
 * @note RTA must know of _all_ definitely executed code parts (main, static sections, global contructors or all nonprivate functions if it's a library)! It's important to give absolutely _all_ entry points because RTA builds on a closed world assumption. Otherwise the results can be incorrect and can lead to defective programs!!
 * @note RTA also won't work with programs that dynamically load classes at runtime! It can lead to defective programs!!
 * @note RTA might also produce incorrect programs if there is some Java Reflections shenanigans in the code (internal or external), especially using java.lang.Class.newInstance()
 * @param n_entry_points number of entry points in array entry_points
 * @param entry_points array of ir_entity*, give all entry points to program code
 */
void rta_optimization(size_t n_entry_points, ir_entity** entry_points);


#endif
