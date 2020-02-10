/*
 * Copyright (C) 1995-2008 University of Karlsruhe.  All right reserved.
 *
 * This file is part of libFirm.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

/**
 * @file
 * @date    10.02.2020
 * @brief   a deterministic pointer to pointer map with a custom compare functions
 * @author  Andreas Fried
 * @version $Id$
 */
#ifndef FIRM_ADT_ARRAYMAP_H
#define FIRM_ADT_ARRAYMAP_H

#include <stddef.h>

/**
 * The type of a arraymap compare function.
 *
 * @param p1   pointer to an element
 * @param p2   pointer to another element
 *
 * @return  1 if the elements are identically, zero else
 */
typedef int (*arraymap_cmp_function) (const void *p1, const void *p2);

typedef struct arraymap_entry_t {
	const void *key;
	void       *data;
} arraymap_entry_t;

typedef struct arraymap_t {
	void **keys;
	void **values;
	arraymap_cmp_function cmp;
} arraymap_t;

typedef struct arraymap_iterator_t {
	const arraymap_t *map;
	size_t index;
} arraymap_iterator_t;

/**
 * Initializes a arraymap
 *
 * @param arraymap           Pointer to allocated space for the arraymap
 * @param hash_function   The hash function to use
 * @param cmp_function    The compare function to use
 */
void arraymap_init(arraymap_t *arraymap, arraymap_cmp_function cmp_function);

/**
 * Destroys a arraymap and frees the memory allocated for hashtable. The memory of
 * the arraymap itself is not freed.
 *
 * @param arraymap   Pointer to the arraymap
 */
void arraymap_destroy(arraymap_t *arraymap);

/**
 * Inserts an element into a arraymap.
 *
 * @param arraymap   Pointer to the arraymap
 * @param key     key under which we file the data
 * @param obj     the data (we just store a pointer to it)
 * @returns       The element itself or a pointer to an existing element
 */
void arraymap_set(arraymap_t *arraymap, void *key, void *data);

/**
 * Removes an element from a arraymap. Does nothing if the arraymap doesn't contain
 * the element.
 *
 * @param arraymap   Pointer to the arraymap
 * @param key     key of the data to remove
 */
void arraymap_remove(arraymap_t *arraymap, const void *key);

/**
 * Tests whether a arraymap contains a pointer
 *
 * @param arraymap   Pointer to the arraymap
 * @param key     Key of the data to find
 * @returns       The data or NULL if not found
 */
void *arraymap_find(const arraymap_t *arraymap, const void *key);

/**
 * Returns the number of pointers contained in the arraymap
 *
 * @param arraymap   Pointer to the arraymap
 * @returns       Number of pointers contained in the arraymap
 */
size_t arraymap_size(const arraymap_t *arraymap);

/**
 * Initializes a arraymap iterator. Sets the iterator before the first element in
 * the arraymap.
 *
 * @param iterator   Pointer to already allocated iterator memory
 * @param arraymap       Pointer to the arraymap
 */
void arraymap_iterator_init(arraymap_iterator_t *iterator, const arraymap_t *arraymap);

/**
 * Advances the iterator and returns the current element or NULL if all elements
 * in the arraymap have been processed.
 * @attention It is not allowed to use arraymap_set or arraymap_remove while
 *            iterating over a arraymap.
 *
 * @param iterator  Pointer to the arraymap iterator.
 * @returns         Next element in the arraymap or NULL
 */
arraymap_entry_t arraymap_iterator_next(arraymap_iterator_t *iterator);

/**
 * Removed the element the iterator currently points to
 *
 * @param arraymap     Pointer to the arraymap
 * @param iterator  Pointer to the arraymap iterator.
 */
void arraymap_remove_iterator(arraymap_t *arraymap, const arraymap_iterator_t *iterator);

#endif
