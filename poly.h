/*
* Copyright 2012, 2013 naehrwert
* Licensed under the terms of the GNU GPL, version 2
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef _POLY_H_
#define _POLY_H_

#include <stdio.h>

#include "types.h"
#include "bignum.h"

/*! Polynomial. */
/*
* Define a polynomial P of degree n as
* P(X) = a_0 + a_1*X + a_2*X^2 + ... + a_n*X^n
* with n + 1 coefficients a_0, a_1, ..., a_n.
*/
typedef struct _poly
{
	/*! The polynomial's degree. */
	u32 degree;
	/*! Coefficients. */
	bn_t **coeffs;
	/*! Modulus. */
	bn_t *N;
} poly_t;

/*!
* \brief Read polynomial from file stream.
*/
poly_t *poly_read(FILE *fp, poly_t *dst);

/*!
* \brief Write polynomial to file stream.
*/
poly_t *poly_write(FILE *fp, poly_t *p);

/*!
* \brief Print polynomial to file stream.
*/
void poly_print(FILE *fp, const s8 *pre, poly_t *p, const s8 *post);

/*!
* \brief Allocate polynomial.
*/
poly_t *poly_alloc(u32 degree, bn_t *N);

/*!
* \brief Free polynomial.
*/
void poly_free(poly_t *p);

/*!
* \brief Set coefficient.
*/
int poly_set_coeff(poly_t *p, u32 i, bn_t *coeff);

/*!
* \brief Free coefficient.
*/
int poly_free_coeff(poly_t *p, u32 i);

/*!
* \brief Evaluate polynomial at given x.
*/
bn_t *poly_eval(poly_t *p, bn_t *dst, bn_t *x);

#endif
