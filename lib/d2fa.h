/*
 * Declaration of delay input deterministic finite automaton.
 *
 * Authors: Zhou Zheng <19950529zz@gmail.com>
 */

#ifndef __D2FA_H
#define __D2FA_H

#include <stdlib.h>
#include "dfa.h"

#define D2FA_FLAG_LAST		(0x01)
#define D2FA_FLAG_DEADEND	(0x02)
#define D2FA_FLAG_ROOT		(0x04)

struct d2fa {
	char	*comment; /* char string with any info */
	size_t	comment_size; /* size of allocated memory */

	size_t 	state_cnt; /* number of dfa states */

	struct tran *trans; /* regular transitions */
	size_t trans_cnt;	/* count of regular transitions*/
	size_t *default_trans; /* array of default transitions */
	size_t default_trans_cnt; /* count of default transitions */
	uint8_t	*flags; /* array of states' flags such as LAST, DEADEND and ROOT*/

	size_t	first_index;	/* initial state's index */
};


struct tran {
	size_t size;
	unsigned char *marks;
	size_t *states;
};

int tran_alloc(struct tran *tran);
int tran_alloc2(struct tran *tran, size_t size);
int tran_free(struct tran *tran);

int d2fa_alloc(struct d2fa *dst);

int d2fa_free(struct d2fa *d2fa);

int d2fa_state_is_root(struct d2fa *, size_t);
int d2fa_state_set_root(struct d2fa *, size_t, int);
int d2fa_state_is_deadend(struct d2fa *, size_t);
int d2fa_state_set_deadend(struct d2fa *, size_t, int);
int d2fa_state_is_last(struct d2fa *, size_t);
int d2fa_state_set_last(struct d2fa *, size_t, int);

int d2fa_add_trans(struct d2fa *, size_t, unsigned char, size_t);

size_t d2fa_get_trans(struct d2fa *, size_t, unsigned char);

/*for debug*/
void d2fa_print(struct d2fa *);

void d2fa_trans_print(struct d2fa *);

void d2fa_default_tran_print(struct d2fa *);

#endif
