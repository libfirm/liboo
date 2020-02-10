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

#include "arrayset.h"

#include <assert.h>
#include <stdint.h>

void arrayset_init(arrayset_t *arrayset, arrayset_cmp_function cmp_function)
{
	arrayset->members = NEW_ARR_F(void*, 0);
	arrayset->cmp     = cmp_function;
}

void arrayset_destroy(arrayset_t *arrayset)
{
	DEL_ARR_F(arrayset->members);
}

void *arrayset_insert(arrayset_t *arrayset, void *obj)
{
	void                        **members = arrayset->members;
	const size_t                  n       = ARR_LEN(members);
	const arrayset_cmp_function   cmp     = arrayset->cmp;

	for (size_t i = 0; i < n; i++) {
		if (members[i] && cmp(members[i], obj)) {
			return members[i];
		}
	}

	// Do not use local variable as ARR_APP1 might realloc
	ARR_APP1(void*, arrayset->members, obj);
	return obj;
}

void arrayset_remove(arrayset_t *arrayset, const void *obj)
{
	void                        **members = arrayset->members;
	const size_t                  n       = ARR_LEN(members);
	const arrayset_cmp_function   cmp     = arrayset->cmp;

	for (size_t i = 0; i < n; i++) {
		if (members[i] && cmp(members[i], obj)) {
			members[i] = NULL;
		}
	}
}

void *arrayset_find(const arrayset_t *arrayset, const void *obj)
{
	void                        **members = arrayset->members;
	const size_t                  n       = ARR_LEN(members);
	const arrayset_cmp_function   cmp     = arrayset->cmp;

	for (size_t i = 0; i < n; i++) {
		if (members[i] && cmp(members[i], obj)) {
			return members[i];
		}
	}

	return NULL;
}

size_t arrayset_size(const arrayset_t *arrayset)
{
	void         **members = arrayset->members;
	const size_t   n       = ARR_LEN(members);

	size_t result = 0;

	for (size_t i = 0; i < n; i++) {
		if (members[i]) {
			result++;
		}
	}

	return result;
}

void arrayset_iterator_init(arrayset_iterator_t *iterator, const arrayset_t *arrayset)
{
	iterator->set = arrayset;
	iterator->index = SIZE_MAX;
}

void *arrayset_iterator_next(arrayset_iterator_t *iterator)
{
	void         **members = iterator->set->members;
	const size_t   n       = ARR_LEN(members);

	do {
		iterator->index++;
	} while (iterator->index < n && members[iterator->index] == NULL);

	if (iterator->index < n) {
		return members[iterator->index];
	} else {
		return NULL;
	}
}

void arrayset_remove_iterator(arrayset_t *arrayset, const arrayset_iterator_t *iterator)
{
	assert(iterator->set == arrayset);
	arrayset->members[iterator->index] = NULL;
}
