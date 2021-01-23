#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "pisim.h"

static long unsigned _unifcg__(long c, unsigned long mult,
                                unsigned long aditiv, unsigned long m) {
	static unsigned long x=0;
	if (c == -1) {
		return x = (mult*x + aditiv) % m;
	} else {
		x = (mult*c + aditiv) % m;
	}
	return ULONG_MAX;
}

double unif(long a, long b, long m) {
	return _unifcg__(-1, a, b, m) / ((double) m-1);
}

void sunif(unsigned long semente, unsigned long a,
		    unsigned long b, unsigned long m) {
	_unifcg__( (long) semente, a, b, m);
}

double calcpi(size_t n, long a, long b, long m, point_t * points) {
	size_t i, cont;
	if (points == NULL) return INFINITY;

	for (i = 0, cont = 0; i < n; i++) {
		points[i].x = 2 * unif(a,b,m) - 1;
		points[i].y = 2 * unif(a,b,m) - 1;

		if (pow(points[i].y, 2) + pow(points[i].x, 2) < 1) {
			cont++;
		}
	}

	return 4 * ( (long double) cont / n );
}

