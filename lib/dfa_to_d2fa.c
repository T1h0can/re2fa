/*
 * Conversion of dfa to d2fa.
 *
 * Authors: Zhou Zheng <19950529zz@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dfa_to_d2fa.h"


void print_graph(size_t **, size_t *);
void print_MST(size_t *, size_t);
void print_ischecked(int8_t *, size_t);

size_t find_root(size_t *arr, size_t index) {
	if (arr[index] == index)
		return index;
	else
		return find_root(arr, arr[index]);
}

void parents_revers(size_t *arr, size_t index) {
	size_t son = index;
	size_t parent = arr[son];
	while (son != parent) {
		size_t tmp = son;
		son = parent;
		parent = arr[son];
		arr[son] = tmp;
	}
	arr[index] = index;
}

int convert_dfa_to_d2fa(struct d2fa *dst, struct dfa *src) {
	dst->state_cnt = src->state_cnt;
	size_t state_cnt = src->state_cnt;
	size_t dead_index = state_cnt;

	size_t **weights = calloc(256, sizeof(size_t *));
	size_t pair_cnt[256] = {0};
	int is_dead_last = 0;

	for (size_t i = 0; i < state_cnt; ++i) {
		if (dfa_state_is_deadend(src, i)){
			dead_index = i;
			if (dfa_state_is_last(src, dead_index))
				is_dead_last = 1;
			break;
		}
	}

	for (size_t i = 0; i < state_cnt - 1; ++i) {
		if (i == dead_index && !is_dead_last)
			continue;
		for (size_t j = i + 1; j < state_cnt; ++j) {
			int weight = 0;
			if (j == dead_index && !is_dead_last)
				continue;
			for (int k = 0; k < 256; ++k) {
				size_t tmp_index_i = dfa_get_trans(src, i, k),
				       tmp_index_j = dfa_get_trans(src, j, k);
				if (tmp_index_i == tmp_index_j) {
					++weight;
				}
			}
			weight -= 1;
			if (weight > 0) {
				weights[weight] = realloc(weights[weight], sizeof(size_t) * 2 * ++pair_cnt[weight]);
				weights[weight][pair_cnt[weight] * 2 - 2] = i;
				weights[weight][pair_cnt[weight] * 2 - 1] = j;
			}
		}
	}

//	print_graph(weights, pair_cnt);	//输出无向图的权重

	size_t *parents = malloc(sizeof(size_t) * state_cnt);
	int8_t *is_checked = calloc( state_cnt, sizeof(int8_t));

	for (size_t i = 0; i < state_cnt; ++i)
		parents[i] = i;
	memset(is_checked, 0, sizeof(int8_t) * state_cnt);

	for (int i = 255; i > 0; --i) {			//构建最大生成树
		for (int j = 0; j < pair_cnt[i]; ++j) {
			size_t m = weights[i][j * 2];
			size_t n = weights[i][j * 2 + 1];

			if (m == dead_index) {
				size_t tmp = m;
				m = n;
				n = tmp;
			}

			if (is_checked[m] && !is_checked[n]) {
				parents[n] = m;
				is_checked[n] = 1;
			} else if(!is_checked[m] && is_checked[n]) {
				parents[m] = n;
				is_checked[m] = 1;
			} else if (!is_checked[m] && !is_checked[n]) {
				parents[n] = m;
				is_checked[m] = is_checked[n] = 1;
			} else {
				size_t root_m = find_root(parents, m);
				size_t root_n = find_root(parents, n);
				if (root_m != root_n){
					if (root_m == dead_index) {
						parents_revers(parents, m);
						parents[m] = n;
					} else  {
						parents_revers(parents, n);
						parents[n] = m;
					}
				}
				else
					continue;
			}
		}
	}

	for (int i = 0; i < 256; ++i)
		free(weights[i]);
	free(weights);

	if (dead_index != state_cnt && dead_index == parents[dead_index])
		parents[dead_index] = src->first_index;	//死状态的父节点为first_index

//	print_MST(parents, state_cnt);

	dst->trans = malloc(sizeof(struct tran) * state_cnt);
	for (size_t i = 0; i < state_cnt; ++i) {
		tran_alloc(&dst->trans[i]);
	}

	for (size_t i = 0; i < state_cnt; ++i) {
		if (i == parents[i]) {
			for (int j = 0; j < 256; ++j)
				d2fa_add_trans(dst, i, j, dfa_get_trans(src, i, j));
			continue;
		}
		if (i == dead_index)
			continue;
		for (int j = 0; j < 256; ++j) {
			size_t tran_ij = dfa_get_trans(src, i, j);
			size_t parent = parents[i];
			if (tran_ij == parent)
				continue;

			int is_need = 1;
			while (1) {
				if(tran_ij == dfa_get_trans(src, parent, j)) {
					is_need = 0;
					break;
				}
				if (parent == parents[parent])
					break;
				parent = parents[parent];
			}

			if (is_need) {
//				printf("%zu-0x%02X->%zu\n", i, j, tran_ij);
				d2fa_add_trans(dst, i, j, tran_ij);
			}
		}
	}

	for (size_t i = 0; i < state_cnt; ++i)
		dst->trans_cnt += dst->trans[i].size;

	dst->comment_size= src->comment_size;
	dst->comment = realloc(dst->comment, dst->comment_size);
	memcpy(dst->comment, src->comment, dst->comment_size);
	dst->first_index = src->first_index;
	dst->flags = malloc(sizeof(int8_t) * state_cnt);
	memcpy(dst->flags, src->flags, sizeof(int8_t) * state_cnt);
	for (size_t i = 0; i < state_cnt; ++i)
		d2fa_state_set_root(dst, i, i == parents[i]);
	dst->default_trans = malloc(sizeof(size_t) * state_cnt);
	memcpy(dst->default_trans, parents, sizeof(size_t) * state_cnt);
	for (size_t i = 0; i < state_cnt; ++i)
		dst->default_trans_cnt += dst->default_trans[i] != i;

	free(parents);
	free(is_checked);

	return 0;
}


/* for debug */
void print_graph(size_t ** weights, size_t *cnt) {
	for(int i = 255; i > 0; --i) {
		if (!cnt[i])
			continue;
		printf("weight[%d]:\n", i);
		for(int j = 0; j < cnt[i]; ++j)
			printf("(%zu--%zu), ", weights[i][j * 2], weights[i][j * 2  + 1]);
		printf("\n");
	}
}

void print_MST(size_t *tree, size_t cnt) {
	for (size_t i = 0; i < cnt; ++i) {
		printf("%zu ", i);
	}
	printf("\n");
	for (size_t i = 0; i < cnt; ++i) {
		printf("%zu ", tree[i]);
	}
	printf("\n");
}

void print_ischecked(int8_t *arr, size_t cnt) {
	for(size_t i = 0; i < cnt; ++i)
		printf("%zu ", i);
	printf("\n");
	for(size_t i = 0; i < cnt; ++i)
		printf("%d ", arr[i]);
	printf("\n");
}
