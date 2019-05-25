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

#ifndef _HPSUDOKU_H_
#define	_HPSUDOKU_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/queue.h>

#include <iostream>

class ELEM;
typedef STAILQ_CLASS_HEAD(ELEM_HEAD,ELEM) ELEM_HEAD_t;
typedef STAILQ_CLASS_ENTRY(ELEM) ELEM_ENTRY_t;

class ELEM {
public:
	ELEM_ENTRY_t entry;
	uint8_t value;
	uint8_t nvar;
	ssize_t var[2];

	enum CMP {
	    CMP_VAR = 1,
	    CMP_VAL = 2,
	    CMP_BOTH = 3,
	};

	ELEM();
	ELEM(const uint8_t);
	ELEM(const uint8_t, const ssize_t);
	ELEM(const uint8_t, const ssize_t, const ssize_t);
	ELEM(const ELEM &);
	~ELEM() {};

	bool isConst() const {
		return (nvar == 0);
	};

	ELEM & expand(const ssize_t);

	int8_t find_var(const ssize_t) const;

	ELEM & operator= (const ELEM &);

	ELEM operator* (const ELEM &other) const {
		ELEM temp(other);
		temp *= *this;
		return (temp);
	};
	ELEM & operator*= (const ELEM &);

	int compare(const ELEM &, enum CMP = CMP_BOTH) const;

	const bool operator> (const ELEM &other) const {
		return (compare(other) > 0);
	};
	const bool operator< (const ELEM &other) const  {
		return (compare(other) < 0);
	};
	const bool operator>= (const ELEM &other) const  {
		return (compare(other) >= 0);
	};
	const bool operator<= (const ELEM &other) const  {
		return (compare(other) <= 0);
	};
	const bool operator== (const ELEM &other) const  {
		return (compare(other) == 0);
	};
	const bool operator!= (const ELEM &other) const  {
		return (compare(other) != 0);
	};

	void print(std::ostream & = std::cout) const;

	void insert_tail(ELEM_HEAD_t *phead) {
		STAILQ_INSERT_TAIL(phead, this, entry);
	};

	void insert_head(ELEM_HEAD_t *phead) {
		STAILQ_INSERT_HEAD(phead, this, entry);
	};

	ELEM *next() const {
		return (STAILQ_NEXT(this, entry));
	};

	void sort() {
		if (nvar == 2) {
			if (var[0] > var[1]) {
				ssize_t temp = var[1];
				var[1] = var[0];
				var[0] = temp;
			} else if (var[0] == var[1]) {
				var[1] = 0;
				nvar = 1;
			} else if (var[0] == -var[1]) {
				var[1] = 0;
				var[0] = 0;
				nvar = 0;
				value = 0;
			}
		}
	};
};

class HEAD {
public:
	HEAD();
	HEAD(const HEAD &);
	~HEAD();

	ELEM_HEAD_t head;
  
	HEAD & expand(const ssize_t);
	HEAD & sort();
	void print(std::ostream & = std::cout) const;

	HEAD & operator= (const HEAD &);

	HEAD operator+ (const HEAD &other) const {
		HEAD temp(other);
		temp += *this;
		return (temp);
	};
	HEAD & operator+= (const HEAD &);

	HEAD operator+ (const ELEM &other) const {
		HEAD temp(*this);
		temp += other;
		return (temp);
	};

	HEAD & operator+= (const ELEM &other) {
		(new ELEM(other))->insert_tail(&head);
		return (*this);
	};
  
	HEAD operator* (const HEAD &other) const {
		HEAD temp(other);
		temp *= *this;
		return (temp);
	};
	HEAD & operator*= (const HEAD &);

	void insert_tail(ELEM *elem) {
		STAILQ_INSERT_TAIL(&head, elem, entry);
	};

	void insert_head(ELEM *elem) {
		STAILQ_INSERT_HEAD(&head, elem, entry);
	};

	bool foreach(ELEM **pptr) const {
		if (*pptr == 0)
			*pptr = STAILQ_FIRST(&head);
		else
			*pptr = STAILQ_NEXT(*pptr, entry);
		return (*pptr != 0);
	};

	void move_to(HEAD *to) {
		ELEM *ptr;

		while ((ptr = to->remove_head()))
			delete ptr;

		while ((ptr = remove_head()))
			ptr->insert_tail(&to->head);
	};

	size_t count() {
		ELEM *ptr;
		size_t retval = 0;
		STAILQ_FOREACH(ptr, &head, entry)
			retval++;
		return (retval);
	};
  
	ELEM *first(void) const {
		return (STAILQ_FIRST(&head));
	};

	uint8_t value(void) const {
		ELEM *ptr = first();
		if (ptr == 0 || ptr->nvar != 0)
			return (0);
		else
			return (ptr->value);
	};

	ELEM *last(void) const {
		return (STAILQ_LAST(&head, ELEM, entry));
	};

	ELEM *remove_head() {
		ELEM *ptr = STAILQ_FIRST(&head);
		if (ptr != 0)
			STAILQ_REMOVE_HEAD(&head, entry);
		return (ptr);
	};

	int compare(const HEAD &) const;

	const bool operator> (const HEAD &other) const {
		return (compare(other) > 0);
	};
	const bool operator< (const HEAD &other) const  {
		return (compare(other) < 0);
	};
	const bool operator>= (const HEAD &other) const  {
		return (compare(other) >= 0);
	};
	const bool operator<= (const HEAD &other) const  {
		return (compare(other) <= 0);
	};
	const bool operator== (const HEAD &other) const  {
		return (compare(other) == 0);
	};
	const bool operator!= (const HEAD &other) const  {
		return (compare(other) != 0);
	};
};

#endif	/* _HPSUDOKU_H_ */
