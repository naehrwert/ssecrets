/*
* Copyright 2012, 2013 naehrwert
* Licensed under the terms of the GNU GPL, version 2
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "bignum.h"
#include "poly.h"
#include "ssecrets.h"

int main(int argc, char **argv)
{
	bn_t *N = bn_from_str(bn_alloc(32), 
		"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F");

	bn_t *s = bn_from_str(bn_alloc(32), 
		"098765432100DEADBEEF000000000000000000000000CAFEBABE001234567890");

	poly_t *p = ssecrets_create_poly(s, 3, N);

	/*poly_t *p = ssecrets_create_poly(NULL, 3, N);
	FILE *fp = fopen("poly.bin", "rb");
	poly_read(fp, p);
	fclose(fp);*/

	bn_t **lx = (bn_t **)malloc(sizeof(bn_t *) * 3);
	bn_t **ls = (bn_t **)malloc(sizeof(bn_t *) * 3);

	u32 i;
	for(i = 0; i < 3; i++)
	{
		lx[i] = bn_reduce(bn_rand(bn_alloc(N->n)), N);
		ls[i] = ssecrets_create_share(p, lx[i]);

		bn_print(stdout, "P(", lx[i], ") = ");
		bn_print(stdout, "", ls[i], "\n");
	}

	bn_t *cs = ssecrets_calc_secret(lx, ls, 3, N);
	bn_print(stdout, "secret = ", cs, "\n");

	getchar();

	return 0;
}
