/*
* Copyright 2012, 2013 naehrwert
* Copyright 2007, 2008, 2010 Segher Boessenkool <segher@kernel.crashing.org>
* Licensed under the terms of the GNU GPL, version 2
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef _BIGNUM_H_
#define _BIGNUM_H_

#include <stdio.h>

#include "types.h"

/*! Comparison results. */
/*! Less. */
#define BN_CMP_L (-1)
/*! Greater. */
#define BN_CMP_G (1)
/*! Equal. */
#define BN_CMP_E (0)

#define BN_INIT(n) bn_zero(bn_alloc(n))

/*! Bignum. */
typedef struct _bn_t
{
	/*! Length. */
	u32 n;
	/*! Value. */
	u8 *v;
} bn_t;

/*!
* \brief Read bignum from file stream.
*/
bn_t *bn_read(FILE *fp, bn_t *dst);

/*!
* \brief Write bignum to file stream.
*/
bn_t *bn_write(FILE *fp, bn_t *num);

/*!
* \brief Print bignum to file stream.
*/
bn_t *bn_print(FILE *fp, const s8 *pre, bn_t *num, const s8 *post);

/*!
* \brief Allocate bignum.
*/
bn_t *bn_alloc(u32 n);

/*!
* \brief Free bignum.
*/
void bn_free(bn_t *num);

/*!
* \brief Fill bignum from string.
*/
bn_t *bn_from_str(bn_t *dst, const s8 *str);

/*!
* \brief Set bignum to zero.
*/
bn_t *bn_zero(bn_t *num);

/*!
* \brief Set bignum to random number.
*/
bn_t *bn_rand(bn_t *num);

/*!
* \brief Copy bignum.
*/
bn_t *bn_copy(bn_t *dst, bn_t *src);

/*!
* \brief Compare two bignums.
*/
s32 bn_compare(bn_t *a, bn_t *b);

/*!
* \brief Reduce bignum.
*/
bn_t *bn_reduce(bn_t *num, bn_t *N);

/*!
* \brief Add two bignums.
*/
bn_t *bn_add(bn_t *dst, bn_t *a, bn_t *b, bn_t *N);

/*!
* \brief Subtract two bignums.
*/
bn_t *bn_sub(bn_t *dst, bn_t *a, bn_t *b, bn_t *N);

/*!
* \brief Multiply two bignums.
*/
bn_t *bn_mon_mul(bn_t *dst, bn_t *a, bn_t *b, bn_t *N);

/*!
* \brief Convert bignum to montgomery form.
*/
bn_t *bn_to_mon(bn_t *num, bn_t *N);

/*!
* \brief Convert bignum from montgomery form.
*/
bn_t *bn_from_mon(bn_t *num, bn_t *N);

/*!
* \brief Exponentiate.
*/
bn_t *bn_mon_exp(bn_t *dst, bn_t *a, bn_t *N, bn_t *e);

/*!
* \brief Invert bignum.
*/
bn_t *bn_mon_inv(bn_t *dst, bn_t *a, bn_t *N);

#endif
