/*
 * Conversion of dfa to d2fa.
 *
 * Authors: Zhou ZHeng <19950529zz@gmail.com>
 */

#ifndef __DFA_TO_D2FA_H
#define __DFA_TO_D2FA_H
#include "dfa.h"
#include "d2fa.h"


/*
 * @1 - pointer to EXISTING INITIALIZED d2fa
 * @2 - pointer to dfa
 * value: 0 if ok
 */
int convert_dfa_to_d2fa(struct d2fa *, struct dfa *);

#endif
