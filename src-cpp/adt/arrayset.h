/*
 * Copyright (C) 2020 University of Karlsruhe.  All right reserved.
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
 * @brief   a set of pointers in deterministic order with a custom compare function
 * @author  Andreas Fried
 */
#ifndef FIRM_ADT_ARRAYSET_H
#define FIRM_ADT_ARRAYSET_H

#include <stddef.h>

#include "array.h"

/**
 * The type of a arrayset compare function.
 *
 * @param p1   pointer to an element
 * @param p2   pointer to another element
 *
 * @return  1 if the elements are identically, zero else
 */
typedef int (*arrayset_cmp_function) (const void *p1, const void *p2);

typedef struct arrayset_t {
	void **members;
	arrayset_cmp_function cmp;
} arrayset_t;

typedef struct arrayset_iterator_t {
	const arrayset_t *set;
	size_t index;
} arrayset_iterator_t;

/**
 * Initializes a arrayset
 *
 * @param arrayset           Pointer to allocated space for the arrayset
 * @param hash_function   The hash function to use
 * @param cmp_function    The compare function to use
 */
void arrayset_init(arrayset_t *arrayset, arrayset_cmp_function cmp_function);

/**
 * Destroys a arrayset and frees the memory allocated for hashtable. The memory of
 * the arrayset itself is not freed.
 *
 * @param arrayset   Pointer to the arrayset
 */
void arrayset_destroy(arrayset_t *arrayset);

/**
 * Inserts an element into a arrayset.
 *
 * @param arrayset   Pointer to the arrayset
 * @param obj     Element to insert into the arrayset
 * @returns       The element itself or a pointer to an existing element
 */
void* arrayset_insert(arrayset_t *arrayset, void *obj);

/**
 * Removes an element from a arrayset. Does nothing if the arrayset doesn't contain the
 * element.
 *
 * @param arrayset   Pointer to the arrayset
 * @param obj     Pointer to remove from the arrayset
 */
void arrayset_remove(arrayset_t *arrayset, const void *obj);

/**
 * Tests whether a arrayset contains a pointer
 *
 * @param arrayset   Pointer to the arrayset
 * @param obj     The pointer to find
 * @returns       An equivalent object to @p obj or NULL
 */
void *arrayset_find(const arrayset_t *arrayset, const void *obj);

/**
 * Returns the number of pointers contained in the arrayset
 *
 * @param arrayset   Pointer to the arrayset
 * @returns       Number of pointers contained in the arrayset
 */
size_t arrayset_size(const arrayset_t *arrayset);

/**
 * Initializes a arrayset iterator. Sets the iterator before the first element in
 * the arrayset.
 *
 * @param iterator   Pointer to already allocated iterator memory
 * @param arrayset       Pointer to the arrayset
 */
void arrayset_iterator_init(arrayset_iterator_t *iterator, const arrayset_t *arrayset);

/**
 * Advances the iterator and returns the current element or NULL if all elements
 * in the arrayset have been processed.
 * @attention It is not allowed to use arrayset_insert or arrayset_remove while
 *            iterating over a arrayset.
 *
 * @param iterator  Pointer to the arrayset iterator.
 * @returns         Next element in the arrayset or NULL
 */
void *arrayset_iterator_next(arrayset_iterator_t *iterator);

/**
 * Removed the element the iterator currently points to
 *
 * @param arrayset     Pointer to the arrayset
 * @param iterator  Pointer to the arrayset iterator.
 */
void arrayset_remove_iterator(arrayset_t *arrayset, const arrayset_iterator_t *iterator);

#endif /* FIRM_ADT_ARRAYSET_H */
