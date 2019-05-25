/*-
 * Copyright (c) 2019 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>

#include "hpsudoku.h"

HEAD :: HEAD()
{
	STAILQ_INIT(&head);
}

HEAD :: HEAD(const HEAD &other)
{
	ELEM *ptr;

	STAILQ_INIT(&head);

	for (ptr = 0; other.foreach(&ptr); )
		(new ELEM(*ptr))->insert_tail(&head);
}

HEAD :: ~HEAD()
{
	ELEM *ptr;

	while ((ptr = remove_head()))
		delete ptr;
}

HEAD &
HEAD :: expand(ssize_t varno)
{
	ELEM *ptr;

	for (ptr = 0; foreach(&ptr); )
		ptr->expand(varno);

	return (sort());
}

HEAD &
HEAD :: operator= (const HEAD &other)
{
	ELEM *ptr;

	if (this == &other)
		return (*this);

	while ((ptr = remove_head()))
		delete ptr;

	for (ptr = 0; other.foreach(&ptr); )
		(new ELEM(*ptr))->insert_tail(&head);
	return (*this);
}

HEAD &
HEAD :: operator+= (const HEAD &other)
{
	ELEM *ptr;

	for (ptr = 0; other.foreach(&ptr); )
		(new ELEM(*ptr))->insert_tail(&head);

	return (sort());
}

HEAD &
HEAD :: operator*= (const HEAD &other)
{
	ELEM *p1;
	ELEM *p2;
	HEAD temp;

	for (p1 = 0; foreach(&p1); ) {
		for (p2 = 0; other.foreach(&p2); ) {
			(new ELEM(*p1 * *p2))->insert_tail(&temp.head);
		}
		temp.sort();
	}

	temp.move_to(this);

	return (*this);
}

void
HEAD :: print(std::ostream &stream) const
{
	ELEM *ptr;

	for (ptr = 0; foreach(&ptr); ) {
		ptr->print(stream);
		stream << "\n";
	}
}

static int
hpsukdoku_sort_compare_elem_var(const void *pa, const void *pb)
{
	const ELEM *ea = *(const ELEM **)pa;
	const ELEM *eb = *(const ELEM **)pb;

	return (ea->compare(*eb, ELEM::CMP_VAR));
}

HEAD &
HEAD :: sort()
{
	ELEM *ptr;
	ELEM **array;
	size_t num;

	for (num = 0, ptr = 0; foreach(&ptr); ) {
		ptr->sort();
		num++;
	}

	if (num == 0)
		return (*this);

	array = new ELEM * [num];

	for (num = 0, ptr = 0; foreach(&ptr); )
		array[num++] = ptr;

	STAILQ_INIT(&head);

	mergesort(array, num, sizeof(array[0]), hpsukdoku_sort_compare_elem_var);

	for (size_t n = 0; n != (num - 1); n++) {
		if (array[n + 1]->compare(*array[n], ELEM::CMP_VAR) == 0) {
			array[n + 1]->value |= array[n]->value;
			delete array[n];
		} else {
			if (array[n]->value != 0)
				array[n]->insert_tail(&head);
			else
				delete array[n];
		}
	}
	if (array[num - 1]->value != 0)
		array[num - 1]->insert_tail(&head);
	else
		delete array[num - 1];

	delete [] array;

	return (*this);
}

int
HEAD :: compare(const HEAD &other) const
{
	ELEM *pa = first();
	ELEM *pb = other.first();

	while (pa && pb) {
		int ret = pa->compare(*pb);
		if (ret)
			return (ret);
		pa = pa->next();
		pb = pb->next();
	}
	return ((pa != 0) - (pb != 0));
}
