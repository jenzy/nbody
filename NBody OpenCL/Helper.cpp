#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "Main.h"

#define M_PI 3.1415926


double checkResults( double *X, double *Y, double *Z, int len ) {
	double x = 0, y = 0, z = 0;
	for( int i = 0; i < len; i++ ) {
		x += X[i];
		y += Y[i];
		z += Z[i];
	}

	double sum = x + y + z;
	printf( "Result check: %lf (%lf, %lf, %lf)\n", sum, x, y, z );
	return sum;
}

/* inicializacija zacetnih polozajev in hitrosti */
/* telesa so na plascu krogle z radijem sphereRadius */
/* hitrost telesa lezi v ravnini, ki je pravokotna na radij */
void generateCoordinates( double *X, double *Y, double *Z, info_t *info ) {
	double fix, fiy, fiz;

	srand( info->seed );
	for( int i = 0; i < info->n; i++ ) {
		fix = 2 * M_PI*rand( ) / (double) RAND_MAX;
		fiy = 2 * M_PI*rand( ) / (double) RAND_MAX;
		fiz = 2 * M_PI*rand( ) / (double) RAND_MAX;

		X[i] = cos( fiz ) * cos( fiy ) * info->sphereRadius;
		Y[i] = -sin( fiz ) * cos( fiy ) * info->sphereRadius;
		Z[i] = -sin( fiy ) * info->sphereRadius;
	}
}