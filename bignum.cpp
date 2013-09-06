/*
* Copyright 2012, 2013 naehrwert
* Copyright 2007, 2008, 2010 Segher Boessenkool <segher@kernel.crashing.org>
* Licensed under the terms of the GNU GPL, version 2
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "bignum.h"
#include "mt19937.h"

static mt19937_ctxt_t _bn_mt_ctxt;
static BOOL _bn_mt_init = FALSE;

static const u8 inv256[0x80] = 
{
	0x01, 0xab, 0xcd, 0xb7, 0x39, 0xa3, 0xc5, 0xef, 
	0xf1, 0x1b, 0x3d, 0xa7, 0x29, 0x13, 0x35, 0xdf, 
	0xe1, 0x8b, 0xad, 0x97, 0x19, 0x83, 0xa5, 0xcf, 
	0xd1, 0xfb, 0x1d, 0x87, 0x09, 0xf3, 0x15, 0xbf, 
	0xc1, 0x6b, 0x8d, 0x77, 0xf9, 0x63, 0x85, 0xaf, 
	0xb1, 0xdb, 0xfd, 0x67, 0xe9, 0xd3, 0xf5, 0x9f, 
	0xa1, 0x4b, 0x6d, 0x57, 0xd9, 0x43, 0x65, 0x8f, 
	0x91, 0xbb, 0xdd, 0x47, 0xc9, 0xb3, 0xd5, 0x7f, 
	0x81, 0x2b, 0x4d, 0x37, 0xb9, 0x23, 0x45, 0x6f, 
	0x71, 0x9b, 0xbd, 0x27, 0xa9, 0x93, 0xb5, 0x5f, 
	0x61, 0x0b, 0x2d, 0x17, 0x99, 0x03, 0x25, 0x4f, 
	0x51, 0x7b, 0x9d, 0x07, 0x89, 0x73, 0x95, 0x3f, 
	0x41, 0xeb, 0x0d, 0xf7, 0x79, 0xe3, 0x05, 0x2f, 
	0x31, 0x5b, 0x7d, 0xe7, 0x69, 0x53, 0x75, 0x1f, 
	0x21, 0xcb, 0xed, 0xd7, 0x59, 0xc3, 0xe5, 0x0f, 
	0x11, 0x3b, 0x5d, 0xc7, 0x49, 0x33, 0x55, 0xff
};

u8 _bn_str_to_u8(const s8 *str)
{
	u32 i = 2;
	u8 t, res = 0;
	s8 c;

	while(i--)
	{
		c = *str++;
		if(c >= '0' && c <= '9')
			t = c - '0';
		else if(c >= 'a' && c <= 'f')
			t = c - 'a' + 10;
		else if(c >= 'A' && c <= 'F')
			t = c - 'A' + 10;
		else
			t = 0;
		res |= t << (i * 4);
	}

	return res;
}

static u8 _bn_add_1(bn_t *dst, bn_t *a, bn_t *b)
{
	u32 i, n, dig;
	u8 c = 0;

	assert(dst->n == a->n && a->n == b->n);

	for(n = dst->n, i = n - 1; i < n; i--)
	{
		dig = a->v[i] + b->v[i] + c;
		c = dig >> 8;
		dst->v[i] = dig;
	}

	return c;
}

static u8 _bn_sub_1(bn_t *dst, bn_t *a, bn_t *b)
{
	u32 i, n, dig;
	u8 c = 1;

	assert(dst->n == a->n && a->n == b->n);

	for(n = dst->n, i = n - 1; i < n; i--)
	{
		dig = a->v[i] + 0xFF - b->v[i] + c;
		c = dig >> 8;
		dst->v[i] = dig;
	}

	return 1 - c;
}

static void _bn_mon_muladd_dig(bn_t *dst, bn_t *a, u8 b, bn_t *N)
{
	u32 i, dig, n;
	u8 z;

	assert(dst->n == a->n && a->n == N->n);

	n = N->n;

	z = -(dst->v[n - 1] + a->v[n - 1] * b) * inv256[N->v[n - 1] / 2];

	dig = dst->v[n - 1] + a->v[n - 1] * b + N->v[n - 1] * z;
	dig >>= 8;

	for(i = n - 2; i < n; i--)
	{
		dig += dst->v[i] + a->v[i] * b + N->v[i] * z;
		dst->v[i + 1] = dig;
		dig >>= 8;
	}

	dst->v[0] = dig;
	dig >>= 8;

	if(dig)
		_bn_sub_1(dst, dst, N);
	
	bn_reduce(dst, N);
}

bn_t *bn_read(FILE *fp, bn_t *dst)
{
	fread(dst->v, sizeof(u8), dst->n, fp);
	return dst;
}

bn_t *bn_write(FILE *fp, bn_t *num)
{
	fwrite(num->v, sizeof(u8), num->n, fp);
	return num;
}

bn_t *bn_print(FILE *fp, const s8 *pre, bn_t *num, const s8 *post)
{
	u32 i, n;

	if(num == NULL)
	{
		fprintf(fp, "%s(NULL)%s", pre, post);
		return NULL;
	}

	fputs(pre, fp);
	for(i = 0, n = num->n; i < n; i++)
		fprintf(fp, "%02X", num->v[i]);
	fputs(post, fp);

	return num;
}

bn_t *bn_alloc(u32 n)
{
	bn_t *res;

	if((res = (bn_t *)malloc(sizeof(bn_t))) == NULL)
		return NULL;

	if((res->v = (u8 *)malloc(sizeof(u8) * n)) == NULL)
	{
		free(res);
		return NULL;
	}

	res->n = n;

	return res;
}

void bn_free(bn_t *num)
{
	if(num == NULL)
		return;

	free(num->v);
	free(num);
}

bn_t *bn_from_str(bn_t *dst, const s8 *str)
{
	u32 i, j, len;

	if(dst == NULL)
		return NULL;
	
	len = strlen(str);
	if(len % 2 != 0)
		return NULL;

	for(i = 0, j = 0; i < len; i += 2, j++)
		dst->v[j] = _bn_str_to_u8(&str[i]);

	return dst;
}

bn_t *bn_zero(bn_t *num)
{
	if(num == NULL)
		return NULL;

	memset(num->v, 0x00, num->n);
	
	return num;
}

bn_t *bn_rand(bn_t *num)
{
	u32 i;

	if(_bn_mt_init == FALSE)
	{
		_bn_mt_init = TRUE;
		mt19937_init(&_bn_mt_ctxt, clock());
	}

	//TODO: use secure PRNG.
	for(i = 0; i < num->n; i++)
		num->v[i] = (u8)(mt19937_update(&_bn_mt_ctxt) & 0xFF);

	return num;
}

bn_t *bn_copy(bn_t *dst, bn_t *src)
{
	if(dst == NULL)
		return NULL;

	assert(dst->n == src->n);

	memcpy(dst->v, src->v, dst->n);

	return dst;
}

s32 bn_compare(bn_t *a, bn_t *b)
{
	u32 i, n;

	assert(a->n == b->n);

	for(i = 0, n = a->n; i < n; i++)
	{
		if(a->v[i] < b->v[i])
			return BN_CMP_L;
		if(a->v[i] > b->v[i])
			return BN_CMP_G;
	}

	return BN_CMP_E;
}

bn_t *bn_reduce(bn_t *num, bn_t *N)
{
	if(num == NULL)
		return NULL;

	assert(num->n == N->n);

	//Reduce until num < N.
	while(bn_compare(num, N) >= 0)
		_bn_sub_1(num, num, N);

	return num;
}

bn_t *bn_add(bn_t *dst, bn_t *a, bn_t *b, bn_t *N)
{
	if(dst == NULL)
		return NULL;

	assert(dst->n == a->n && a->n == b->n && b->n == N->n);

	if(_bn_add_1(dst, a, b))
		_bn_sub_1(dst, dst, N);

	bn_reduce(dst, N);

	return dst;
}

bn_t *bn_sub(bn_t *dst, bn_t *a, bn_t *b, bn_t *N)
{
	if(dst == NULL)
		return NULL;

	assert(dst->n == a->n && a->n == b->n && b->n == N->n);

	if(_bn_sub_1(dst, a, b))
		_bn_add_1(dst, dst, N);

	return dst;
}

bn_t *bn_mon_mul(bn_t *dst, bn_t *a, bn_t *b, bn_t *N)
{
	u32 i, n;
	bn_t *t;

	if(dst == NULL)
		return NULL;

	assert(dst->n == a->n && a->n == b->n && b->n == N->n);

	t = bn_zero(bn_alloc(dst->n));

	for(n = dst->n, i = n - 1; i < n; i--)
		_bn_mon_muladd_dig(t, a, b->v[i], N);

	bn_copy(dst, t);

	bn_free(t);

	return dst;
}

bn_t *bn_to_mon(bn_t *num, bn_t *N)
{
	u32 i, n;

	if(num == NULL)
		return NULL;

	assert(num->n == N->n);

	for(i = 0, n = 8 * num->n; i < n; i++)
		bn_add(num, num, num, N);

	return num;
}

bn_t *bn_from_mon(bn_t *num, bn_t *N)
{
	bn_t *t;

	if(num == NULL)
		return NULL;

	assert(num->n == N->n);

	t = bn_zero(bn_alloc(num->n));
	t->v[num->n - 1] = 1;
	bn_mon_mul(num, num, t, N);
	bn_free(t);

	return num;
}

bn_t *bn_mon_exp(bn_t *dst, bn_t *a, bn_t *N, bn_t *e)
{
	u32 i;
	u8 mask;
	bn_t *t, *s;

	if(dst == NULL)
		return NULL;

	assert(dst->n == a->n && a->n == N->n);

	t = bn_alloc(dst->n);
	s = bn_zero(bn_alloc(dst->n));

	s->v[dst->n - 1] = 1;
	bn_to_mon(s, N);

	for(i = 0; i < e->n; i++)
		for(mask = 0x80; mask != 0; mask >>= 1)
		{
			bn_mon_mul(t, s, s, N);
			if((e->v[i] & mask) != 0)
				bn_mon_mul(s, t, a, N);
			else
				bn_copy(s, t);
		}

	bn_copy(dst, s);

	bn_free(s);
	bn_free(t);

	return dst;
}

bn_t *bn_mon_inv(bn_t *dst, bn_t *a, bn_t *N)
{
	bn_t *t, *s;

	if(dst == NULL)
		return NULL;

	assert(dst->n == a->n && a->n == N->n);

	t = bn_alloc(dst->n);
	s = bn_zero(bn_alloc(dst->n));

	s->v[dst->n - 1] = 2;
	_bn_sub_1(t, N, s);
	bn_mon_exp(dst, a, N, t);

	bn_free(s);
	bn_free(t);

	return dst;
}
