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

ELEM :: ELEM(void)
{
	value = 0;
	nvar = 0;
	var[0] = 0;
	var[1] = 0;
}

ELEM :: ELEM(const ELEM &other)
{
	value = 0;
	nvar = 0;
	var[0] = 0;
	var[1] = 0;

	*this = other;
}

ELEM :: ELEM(const uint8_t _value)
{
	value = _value;
	nvar = 0;
}

ELEM :: ELEM(const uint8_t _value, const ssize_t v0)
{
	value = _value;
	nvar = 1;
	var[0] = v0;
	var[1] = 0;
}

ELEM :: ELEM(const uint8_t _value, const ssize_t v0, const ssize_t v1)
{
	value = _value;
	nvar = 2;
	var[0] = v0;
	var[1] = v1;

	sort();
}

ELEM &
ELEM :: operator=(const ELEM &other)
{
	if (&other == this)
		return (*this);

	nvar = other.nvar;
	var[0] = other.var[0];
	var[1] = other.var[1];
	value = other.value;

	sort();
	return (*this);
}

ELEM &
ELEM :: operator*= (const ELEM &other)
{
	uint8_t x;

	for (x = 0; x != other.nvar; x++) {
		if (find_var(other.var[x]) >= 0)
			continue;
		if (find_var(-other.var[x]) >= 0 || nvar == 2) {
			value = 0;
			if (nvar == 2)
				printf("ERROR\n");
			break;
		}
		var[nvar++] = other.var[x];
	}
	value &= other.value;

	sort();
	return (*this);
}

int
ELEM :: compare(const ELEM &other, enum CMP cmp) const
{

	if (cmp & CMP_VAR) {
		if (nvar > other.nvar)
			return (1);
		else if (nvar < other.nvar)
			return (-1);

		switch (nvar) {
		case 2:
			if (var[1] > other.var[1])
				return (1);
			else if (var[1] < other.var[1])
				return (-1);
			/* FALLTHROUGH */
		case 1:
			if (var[0] > other.var[0])
				return (1);
			else if (var[0] < other.var[0])
				return (-1);
			/* FALLTHROUGH */
		default:
			break;
		}
	}

	if (cmp & CMP_VAL) {
		if (value > other.value)
			return (1);
		else if (value < other.value)
			return (-1);
	}
	return (0);
}

int8_t
ELEM :: find_var(const ssize_t varno) const
{
	for (uint8_t x = 0; x != nvar; x++) {
		if (var[x] == varno)
			return (x);
	}
	return (-1);
}

ELEM &
ELEM :: expand(const ssize_t varno)
{
	int8_t nor = find_var(varno);
	int8_t inv = find_var(-varno);

	if (nor >= 0) {
		if (nvar == 2) {
			if (nor == 0)
				var[0] = var[1];
			var[1] = 0;
			nvar = 1;
		} else {
			var[0] = 0;
			nvar = 0;
		}
	} else if (inv >= 0) {
		value = 0;
		nvar = 0;
		var[0] = 0;
		var[1] = 0;
	}
	return (*this);
}

void
ELEM :: print(std::ostream &stream) const
{

	if (isConst()) {
		stream << (int)value;
	} else {
		switch (nvar) {
		case 1:
			stream << (int)value << "*{" << var[0] << "}";
			break;
		case 2:
			stream << (int)value << "*{" << var[0] << "&" << var[1] << "}";
			break;
		default:
			break;
		}
	}
}
