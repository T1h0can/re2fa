/*
 * Definition of delay input deterministic finite automaton.
 *
 * Authors: Zhou Zheng <19950529zz@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "d2fa.h"

int tran_alloc(struct tran *tran) {
	tran->size = 0;
	tran->marks = NULL;
	tran->states = NULL;
	return 0;
}

int tran_alloc2(struct tran *tran, size_t size) {
	tran->size = size;
	tran->marks = malloc(sizeof(unsigned char) * size);
	tran->states = malloc(sizeof(size_t) * size);
	return 0;
}

int tran_free(struct tran *tran) {
	if (tran != NULL){
		free(tran->marks);
		free(tran->states);
	}
	return 0;
}

int d2fa_alloc(struct d2fa *dst) {
	dst->comment_size = 0;
	dst->comment = NULL;
	dst->state_cnt = 0;
	dst->trans = NULL;
	dst->trans_cnt = 0;

	dst->default_trans = NULL;
	dst->default_trans_cnt = 0;
	dst->flags = NULL;

	dst->first_index = 0;
	dst->dead_index = ~0;
	return 0;
}

int d2fa_free(struct d2fa *d2fa) {
	if (d2fa != NULL) {
		for (size_t i = 0; i < d2fa->state_cnt; ++i)
			tran_free(&d2fa->trans[i]);
		free(d2fa->trans);
		free(d2fa->default_trans);
		free(d2fa->flags);
		free(d2fa->comment);
	}
	return 0;
}

int d2fa_state_is_root(struct d2fa * d2fa, size_t state) {
	if (state > d2fa->state_cnt)
		return -1;
	if (d2fa->flags[state] & D2FA_FLAG_ROOT)
		return 1;
	else
		return 0;
}

int d2fa_state_set_root(struct d2fa * d2fa, size_t state, int root) {
	if (state > d2fa->state_cnt)
		return -1;

	if (root)
		d2fa->flags[state] |= D2FA_FLAG_ROOT;
	else
		d2fa->flags[state] &= 0xFF ^ D2FA_FLAG_ROOT;

	return 0;
}

int d2fa_state_is_last(struct d2fa *d2fa, size_t state)
{
	if (d2fa->flags[state] & D2FA_FLAG_LAST)
		return 1;
	else
		return 0;
}

int d2fa_state_set_last(struct d2fa *d2fa, size_t state, int last)
{
	if (state > d2fa->state_cnt)
		return -1;

	if (last)
		d2fa->flags[state] |= D2FA_FLAG_LAST;
	else
		d2fa->flags[state] &= 0xFF ^ D2FA_FLAG_LAST;

	return 0;
}

int d2fa_state_is_deadend(struct d2fa *d2fa, size_t state)
{
	if (state > d2fa->state_cnt)
		return -1;

	if (d2fa->flags[state] & D2FA_FLAG_DEADEND)
		return 1;
	else
		return 0;
}

int d2fa_state_set_deadend(struct d2fa *d2fa, size_t state, int deadend)
{
	if (state > d2fa->state_cnt)
		return -1;

	if (deadend)
		d2fa->flags[state] |= D2FA_FLAG_DEADEND;
	else
		d2fa->flags[state] &= 0xFF ^ D2FA_FLAG_DEADEND;

	return 0;
}

int d2fa_add_trans(struct d2fa *d2fa, size_t from, unsigned char mark, size_t to) {
	struct tran *tmp = &d2fa->trans[from];
	tmp->marks = realloc(tmp->marks, sizeof(unsigned char) * (tmp->size + 1));
	tmp->states = realloc(tmp->states, sizeof(size_t) * (tmp->size + 1));
	tmp->marks[tmp->size] = mark;
	tmp->states[tmp->size] = to;
	++ tmp->size;
	return 0;
}

size_t d2fa_get_trans(struct d2fa *d2fa, size_t from, unsigned char mark) {
	for (size_t i = 0; i < d2fa->trans[from].size; ++i)
		if(d2fa->trans->marks[i] == mark)
			return d2fa->trans->states[i];
	return -1;
}

size_t get_parent(struct d2fa *d2fa, size_t state) {
	if (state > d2fa->state_cnt)
		return -1;

	return d2fa->default_trans[state];
}

void d2fa_print_char(unsigned char a)
{
	if (isprint(a)) {
		if (a == '"' || a == '-' || a == '\\')
			printf("\\");
		printf("%c", a);
	} else {
		printf("\\x%02X", a);
	}
}

void d2fa_print(struct d2fa *src)
{
	printf("#d2fa states: %zu, first: %zu\n", src->state_cnt, src->first_index);

	for (size_t i = 0; i < src->state_cnt; i++) {
		printf("{node [shape = %s, %s%s%s%s]; \"%zu\";}\n",
			   (d2fa_state_is_last(src, i) ? "doublecircle" : "circle"),
			   ((i == src->first_index || i == src->default_trans[i] || i == src->dead_index) ? "style=" : ""),
			   (i == src->first_index ? "bold, " : ""),
			   (i == src->default_trans[i] ? "root ": ""),
			   (i == src->dead_index ? "dead " : ""),
			   i);
	}

	for (size_t i = 0; i < src->state_cnt; i++) {
		printf("#node[%zu] %zu trans\n", i, src->trans[i].size);
		if (i != src->default_trans[i]){
			printf(" \"%zu\" -> \"%zu\" [default tran];\n", i, src->default_trans[i]);
		}

		size_t size = src->trans[i].size;
		if (size == 0)
			continue;
		struct to_and_symb {size_t to; unsigned char symb;}	qu[size];

		for (unsigned int j = 0; j < size; j++) {
			qu[j].to = src->trans[i].states[j];
			qu[j].symb = src->trans[i].marks[j];
		}

		for (unsigned int j = 0; j < size - 1; j++)
			for (unsigned int k = 0; k < size - 1 - j; k++) {
				if (qu[k].to > qu[k + 1].to ||
				    (qu[k].to == qu[k + 1].to && qu[k].symb > qu[k + 1].symb)) {

					struct to_and_symb	tmp;
					tmp = qu[k];
					qu[k] = qu[k + 1];
					qu[k + 1] = tmp;
				}
			}

		for (unsigned int j = 0; j < size; j++) {
			int	symbs[256];
			memset(symbs, 0u, sizeof(symbs));
			unsigned int d_bnd, u_bnd;
			unsigned int total_trans = 0;
			d_bnd = j;
			for (u_bnd = j; u_bnd < 256 && qu[d_bnd].to == qu[u_bnd].to; u_bnd++) {
				symbs[qu[u_bnd].symb] = 1;
				total_trans++;
			}
			if (total_trans > 256/2 && total_trans != size) {
				for (unsigned int j = 0; j < 256; j++)
					symbs[j] = 1 - symbs[j];
			}

			unsigned int	ub, db;
			int	started = 0;
			printf(" \"%zu\" -> \"%zu\" [label = \"", i, qu[d_bnd].to);
			if (total_trans > 1)
				printf("[");
			if (total_trans > 256/2 && total_trans != 256)
				printf("^");
			for (db = 0; db < 256;)
				if (symbs[db] == 0) {
					db++;
				} else {
					for (ub = db; ub < 256 && symbs[ub]; ub++);

					ub--;
					if (db == ub) {
						d2fa_print_char(db);
					} else if (ub - db == 1) {
						d2fa_print_char(db);
						d2fa_print_char(ub);
					} else {
						d2fa_print_char(db);
						printf("-");
						if (isprint(ub)) {
							if (ub == '\\' || ub == '"' || ub == ']')
								printf("\\");
							printf("%c", ub);
						} else {
							printf("\\x%02X", ub);
						}
					}

					if (!started) {
						started = 1;
					} else {
					}
					db = ub + 1;
				}

			j = u_bnd - 1;
			if (total_trans > 1)
				printf("]");
			printf("\"];\n");
		}
	}
}

void d2fa_trans_print(struct d2fa *src) {
	for (size_t i = 0; i < src->state_cnt; ++i) {
		size_t size = src->trans[i].size;
		printf("%zu(%zu):\n", i, size);
		for (size_t j = 0; j < size; ++j)
			printf("0x%02X->%zu ", src->trans[i].marks[j], src->trans[i].states[j]);
		if(size)
			printf("\n");
	}
}


void d2fa_default_tran_print(struct d2fa *src) {
	printf("[D2FA] default trans root:");
	for (size_t i = 0; i < src->state_cnt; ++i)
		if (d2fa_state_is_root(src, i))
			printf("%zu, ", i);
	printf("\n");
	for (size_t i = 0; i < src->state_cnt; ++i) {
		if (i != src->default_trans[i]){
			printf("#node[%zu] -> \"%zu\"\n", i, src->default_trans[i]);
		}
	}

}
