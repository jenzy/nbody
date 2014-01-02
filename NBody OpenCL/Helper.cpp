#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "Main.h"

#define M_PI 3.1415926


double checkResults( double *X, double *Y, double *Z, int len ) {
	double sum = 0;
	double *sums = (double*) malloc(len * sizeof(double));
	for( int i = 0; i < len; i++ ) {
		sum += X[i] + Y[i] + Z[i];
		sums[i] = X[i] + Y[i] + Z[i];
	}
	printf( "Result check: %lf [", sum );
	for( int i = 0; i < 3; i++ ) {
		printf( "%lf ", sums[i] );
	}
	printf( "]\n" );
	free( sums );
	return sum;
}
float checkResults( float *X, float *Y, float *Z, int len ) {
	float sum = 0;
	float *sums = (float*) malloc( len * sizeof(float) );
	for( int i = 0; i < len; i++ ) {
		sum += X[i] + Y[i] + Z[i];
		sums[i] = X[i] + Y[i] + Z[i];
	}
	printf( "Result check: %f [", sum );
	
	for( int i = 0; i < 10; i++ ) {
		printf( "%f ", sums[i] );
	}
	printf( "]\n" );
	free( sums );
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
void generateCoordinates( float *X, float *Y, float *Z, info_t *info ) {
	float fix, fiy, fiz;

	srand( info->seed );
	for( int i = 0; i < info->n; i++ ) {
		fix = 2 * M_PI*rand( ) / (float) RAND_MAX;
		fiy = 2 * M_PI*rand( ) / (float) RAND_MAX;
		fiz = 2 * M_PI*rand( ) / (float) RAND_MAX;

		X[i] = cos( fiz ) * cos( fiy ) * info->sphereRadius;
		Y[i] = -sin( fiz ) * cos( fiy ) * info->sphereRadius;
		Z[i] = -sin( fiy ) * info->sphereRadius;
	}
}