/*
* Copyright 2012, 2013 naehrwert
* Licensed under the terms of the GNU GPL, version 2
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include <stdlib.h>

#include "poly.h"

poly_t *poly_read(FILE *fp, poly_t *dst)
{
	u32 i;

	for(i = 0; i <= dst->degree; i++)
		bn_read(fp, dst->coeffs[i]);

	return dst;
}

poly_t *poly_write(FILE *fp, poly_t *p)
{
	u32 i;

	for(i = 0; i <= p->degree; i++)
		bn_write(fp, p->coeffs[i]);

	return p;
}

void poly_print(FILE *fp, const s8 *pre, poly_t *p, const s8 *post)
{
	u32 i;

	if(p == NULL)
	{
		fprintf(fp, "%s(NULL)%s", pre, post);
		return;
	}

	fputs(pre, fp);
	for(i = 0; i <= p->degree; i++)
	{
		bn_print(fp, "", bn_from_mon(p->coeffs[i], p->N), "");
		bn_to_mon(p->coeffs[i], p->N);
		if(i > 0)
		{
			if(i > 1)
				fprintf(fp, "*X^%d", i);
			else
				fputs("*X", fp);
		}
		if(i < p->degree)
			fputs(" + ", fp);
	}
	fputs(post, fp);
}

poly_t *poly_alloc(u32 degree, bn_t *N)
{
	poly_t *res;

	if((res = (poly_t *)malloc(sizeof(poly_t))) == NULL)
		return NULL;
	
	//Degree + 1 coeffs.
	if((res->coeffs = (bn_t **)malloc(sizeof(bn_t *) * (degree + 1))) == NULL)
	{
		free(res);
		return NULL;
	}

	res->degree = degree;
	res->N = N;

	return res;
}

void poly_free(poly_t *p)
{
	free(p->coeffs);
	free(p);
}

int poly_set_coeff(poly_t *p, u32 i, bn_t *coeff)
{
	if(i > p->degree)
		return 0;

	//Convert coefficients for faster eval.
	p->coeffs[i] = bn_to_mon(coeff, p->N);
	return 1;
}

int poly_free_coeff(poly_t *p, u32 i)
{
	if(i > p->degree)
		return 0;

	bn_free(p->coeffs[i]);
	p->coeffs[i] = NULL;
	return 1;
}

bn_t *poly_eval(poly_t *p, bn_t *dst, bn_t *x)
{
	u32 i;
	bn_t *e = BN_INIT(4), *t = BN_INIT(p->N->n), *tx = bn_to_mon(bn_copy(BN_INIT(x->n), x), p->N);


	for(i = 0; i <= p->degree; i++)
	{
		//TODO: endianess!!!
		e->v[3] = i & 0x000000FF;
		e->v[2] = (i & 0x0000FF00) >> 8;
		e->v[1] = (i & 0x00FF0000) >> 16;
		e->v[0] = (i & 0xFF000000) >> 24;

		//t = x^e
		bn_mon_exp(t, tx, p->N, e);
		//dst += t * a_i
		bn_add(dst, dst, bn_from_mon(bn_mon_mul(t, t, p->coeffs[i], p->N), p->N), p->N);
	}

	bn_free(e);
	bn_free(t);
	bn_free(tx);

	return dst;
}
