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

#include <err.h>

#include "hpsudoku.h"

static void
hpsudoku_solve(HEAD & head, ssize_t *v_result, ssize_t v_start, ssize_t num)
{
	HEAD *h_result = new HEAD[num];

	for (ssize_t x = 0; x != num; v_start++, x++) {
		ELEM *e;

		HEAD t;
		HEAD f;
		HEAD r;

		while ((e = head.remove_head())) {
			if (e->find_var(v_start) >= 0)
				e->insert_tail(&t.head);
			else if (e->find_var(-v_start) >= 0)
				e->insert_tail(&f.head);
			else
				e->insert_tail(&r.head);
		}

		t.expand(v_start);
		f.expand(-v_start);

		head = r + (t * f);

		t.move_to(&h_result[x]);
	}

	while (num--) {
		v_start--;

		if (h_result[num].value() == 0)
			v_result[num] = v_start;
		else
			v_result[num] = -v_start;

		for (ssize_t x = 0; x != num; x++)
			h_result[x].expand(v_result[num]);
	}

	delete[] h_result;
}

static ssize_t
hpsudoku_variable(size_t size, size_t x, size_t y, size_t z)
{

	return (x * size * size + y * size + z + 1);
}

static void
hpsudoku_read(size_t size, const char *ptr)
{
	ssize_t map[size][size];
	ssize_t result[size][size][size];
	size_t num = size * size;
	size_t x;
	size_t y;
	size_t z;
	size_t t;
	size_t u;
	HEAD head;

	memset(map, 0, sizeof(map));

	for (x = 0; x != num; x++) {
		if (ptr[x] == 0)
			errx(1, "Too few numbers in line");

		switch (ptr[x]) {
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			map[0][x] = ptr[x] - '0';
			break;
		default:
			break;
		}
	}

	while (ptr[x] == ' ' || ptr[x] == '\t' || ptr[x] == '\r' || ptr[x] == '\n')
		x++;
	if (ptr[x] != 0)
		errx(1, "Extra characters at end of line");

	/* build generic equation */

	/* only one value selection is allowed per variable */
	for (x = 0; x != size; x++) {
		for (y = 0; y != size; y++) {
			for (z = 0; z != size; z++) {
				for (t = z + 1; t != size; t++) {
					/* add 2-SAT rule */
					(new ELEM(1,
					    hpsudoku_variable(size, x, y, z),
					    hpsudoku_variable(size, x, y, t)))->insert_tail(&head.head);
				}
			}
		}
	}

	/* inside a 3x3 box same value may only appear once */
	for (u = 0; u != size; u++) {
		for (x = 0; x != size; x++) {
			size_t a = x - (x % 3);

			for (y = 0; y != size; y++) {
				size_t b = y - (y % 3);

				for (z = 0; z != 3; z++) {
					size_t c = a + z;

					for (t = 0; t != 3; t++) {
						size_t d = b + t;

						if (c == x && d == y)
							continue;

						/* add 2-SAT rule */
						(new ELEM(1,
						    hpsudoku_variable(size, x, y, u),
						    hpsudoku_variable(size, c, d, u)))->insert_tail(&head.head);
					}
				}
			}
		}
	}

	/* inside each row or column value may only appear once */
	for (u = 0; u != size; u++) {
		for (x = 0; x != size; x++) {
			for (y = 0; y != size; y++) {
				for (t = 0; t != size; t++) {
					if (t == x)
						continue;
					/* add 2-SAT rule */
					(new ELEM(1,
					    hpsudoku_variable(size, x, y, u),
					    hpsudoku_variable(size, t, y, u)))->insert_tail(&head.head);
					(new ELEM(1,
					    hpsudoku_variable(size, y, x, u),
					    hpsudoku_variable(size, y, t, u)))->insert_tail(&head.head);
				}
			}
		}
	}

	/* expand known values */
	for (x = 0; x != size; x++) {
		for (y = 0; y != size; y++) {
			if (map[x][y] == 0)
				continue;
			for (z = 0; z != size; z++) {
				head.expand(((z + 1) == map[x][y]) ?
				    hpsudoku_variable(size, x, y, z) :
				    -hpsudoku_variable(size, x, y, z));
			}
		}
	}

	/* make sure input is sorted */
	head.sort();

	hpsudoku_solve(head, result[0][0], 1, size * size * size);

	/* expand known values */
	for (x = 0; x != size; x++) {
		for (y = 0; y != size; y++) {
			if (map[x][y] != 0)
				continue;
			for (z = 0; z != size; z++) {
				if (result[x][y][z] > 0) {
					if (map[x][y] == 0)
						map[x][y] = z + 1;
					else
						errx(1, "Unsolvable");
				}
			}
			if (map[x][y] == 0)
				errx(1, "Unsolvable");
		}
	}

	/* print result */
	for (x = 0; x != size; x++) {
		if (x != 0 && (x % 3) == 0) {
			for (y = 0; y != size; y++) {
				if (y != 0 && (y % 3) == 0)
					printf("+-");
				printf("--");
			}
			printf("\n");
		}
		for (y = 0; y != size; y++) {
			if (y != 0 && (y % 3) == 0)
				printf("| ");
			printf("%zd ", map[x][y]);
		}
		printf("\n");
	}
	printf("\n");
}

int
main(int argc, char **argv)
{
	char buffer[256];
	int ch;
	int off = 0;

	while ((ch = getchar()) != -1) {
		if (off == 255 || ch == '\n') {
			buffer[off] = 0;
			off = 0;
			hpsudoku_read(9, buffer);
		} else {
			buffer[off++] = ch;
		}
	}
	return (0);
}
