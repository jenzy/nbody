#include <cstdio>

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