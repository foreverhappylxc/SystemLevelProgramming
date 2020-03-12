#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "cache.h"


/* Here is an our naive implementation */
char rotate_descr[] = "Naive Row-wise Traversal of src";
void rotate(int dim, pixel *src, pixel *dst) {
	int i, j, m, n;
	int juzheng = 4;

	for (n = 0; n < dim; n += juzheng)
		for (m = 0; m < dim; m += juzheng)
			for (j = n + juzheng - 1; j >= n; j--)
				for (i = m; i < m + juzheng; i++)
					COPY(&dst[PIXEL(dim-1-j,i,dim)], &src[PIXEL(i,j,dim)]);
	return;
}


/* Add additional functions to test here */
void register_rotate_functions() {
	add_rotate_function(&rotate, rotate_descr);
	
}

