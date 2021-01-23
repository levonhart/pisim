#include <stdio.h>
#include <stdlib.h>
#include "pisim.h"

int main(int argc, char *argv[]) {
	long a = MULTIP,
		 b = ADITIV,
		 m = MODULO;
	sunif(123456, a, b, m);
	size_t n = NPOINT;
	point_t v[n];

	double pi = calcpi(n,a,b,m,v);
	printf("%.15lf\n", pi);

	return EXIT_SUCCESS;
}
