#ifndef PISIM_H_IUXZOJDE
#define PISIM_H_IUXZOJDE

#define MODULO 0x0FFFFFFFFFFFFFFFLU
#define MULTIP 0x0000969696969601LU // 4* 0x9696969696 + 1
#define ADITIV 0x00800AAAAAAAAAABLU

#define NPOINT 400000

static long unsigned _unifcg__(long c, unsigned long mult,
                                unsigned long aditiv, unsigned long m);

double unif(long a, long b, long m);

void sunif(unsigned long semente, unsigned long a,
		    unsigned long b, unsigned long m);

typedef struct _point_t {
	double x, y;
} point_t;


double calcpi(size_t n, long a, long b, long m, point_t * points);


#endif /* end of include guard: PISIM_H_IUXZOJDE */
