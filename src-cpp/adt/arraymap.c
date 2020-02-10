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
 * @brief   Custom pointer set
 * @author  Andreas Fried
 *
 * This implements a deterministic set of pointers which allows to
 * specify custom callbacks for comparing its elements.
 */
#include "arraymap.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "array.h"

void arraymap_init(arraymap_t *arraymap, arraymap_cmp_function cmp_function)
{
	arraymap->keys   = NEW_ARR_F(void*, 0);
	arraymap->values = NEW_ARR_F(void*, 0);
	arraymap->cmp    = cmp_function;
}

void arraymap_destroy(arraymap_t *arraymap)
{
	DEL_ARR_F(arraymap->keys);
	DEL_ARR_F(arraymap->values);
}

void arraymap_set(arraymap_t *arraymap, void *key, void *data)
{
	if (data == NULL) {
		printf("Error: arraymap_set used to delete an element\n");
		assert(false);
	}

	void                  **keys = arraymap->keys;
	size_t                  n    = ARR_LEN(keys);
	arraymap_cmp_function   cmp  = arraymap->cmp;

	for (size_t i = 0; i < n; i++) {
		if (keys[i] && cmp(keys[i], key)) {
			arraymap->values[i] = data;
			return;
		}
	}

	// Do not use local variable as ARR_APP1 might realloc
	ARR_APP1(void*, arraymap->keys,   key);
	ARR_APP1(void*, arraymap->values, data);
}

void arraymap_remove(arraymap_t *arraymap, const void *key)
{
	void                  **keys = arraymap->keys;
	size_t                  n    = ARR_LEN(keys);
	arraymap_cmp_function   cmp  = arraymap->cmp;

	for (size_t i = 0; i < n; i++) {
		if (keys[i] && cmp(keys[i], key)) {
			arraymap->keys[i]   = NULL;
			arraymap->values[i] = NULL;
			return;
		}
	}
}

void *arraymap_find(const arraymap_t *arraymap, const void *key)
{
	void                  **keys = arraymap->keys;
	size_t                  n    = ARR_LEN(keys);
	arraymap_cmp_function   cmp  = arraymap->cmp;

	for (size_t i = 0; i < n; i++) {
		if (keys[i] && cmp(keys[i], key)) {
			return arraymap->values[i];
		}
	}

	return NULL;
}

size_t arraymap_size(const arraymap_t *arraymap)
{
	void   **keys = arraymap->keys;
	size_t   n    = ARR_LEN(keys);

	size_t result = 0;

	for (size_t i = 0; i < n; i++) {
		if (keys[i]) {
			result++;
		}
	}

	return result;
}

void arraymap_iterator_init(arraymap_iterator_t *iterator, const arraymap_t *arraymap)
{
	iterator->map = arraymap;
	iterator->index = SIZE_MAX;
}

arraymap_entry_t arraymap_iterator_next(arraymap_iterator_t *iterator)
{
	void         **keys = iterator->map->keys;
	const size_t   n    = ARR_LEN(keys);

	do {
		iterator->index++;
	} while (iterator->index < n && keys[iterator->index] == NULL);

	if (iterator->index < n) {
		return (arraymap_entry_t) {
			.key  = keys[iterator->index],
			.data = iterator->map->values[iterator->index]
		};
	}

	return (arraymap_entry_t) {
		.key  = NULL,
		.data = NULL
	};
}

void arraymap_remove_iterator(arraymap_t *arraymap, const arraymap_iterator_t *iterator)
{
	assert(iterator->map == arraymap);

	arraymap->keys  [iterator->index] = NULL;
	arraymap->values[iterator->index] = NULL;
}
