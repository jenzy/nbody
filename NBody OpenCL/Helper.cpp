#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "Main.h"

#define CHECK_N 5

float checkResults( float *X, float *Y, float *Z, int len ) {
	float sum = 0;
	float *sums = (float*) malloc( len * sizeof(float) );
	for( int i = 0; i < len; i++ ) {
		sums[i] = X[i] + Y[i] + Z[i];
		sum += sums[i];
	}
	printf( "Result check: %f [", sum );
	
	for( int i = 0; i < CHECK_N; i++ ) {
		printf( "%f ", sums[i] );
	}
	printf( "]\n" );
	free( sums );
	return sum;
}

float checkResultsFloat4( float *coord, int len ) {
	float sum = 0;
	float *sums = (float*) malloc( len * sizeof(float) );
	for( int i = 0; i < len; i++ ) {
		int index = 4 * i;
		float x = coord[index];
		float y = coord[index + 1];
		float z = coord[index + 2];
		sums[i] = x + y + z;
		sum += sums[i];
	}
	printf( "Result check: %f [", sum );

	for( int i = 0; i < CHECK_N; i++ ) {
		printf( "%f ", sums[i] );
	}
	printf( "]\n" );
	free( sums );
	return sum;
}

/* inicializacija zacetnih polozajev in hitrosti */
/* telesa so na plascu krogle z radijem sphereRadius */
/* hitrost telesa lezi v ravnini, ki je pravokotna na radij */

void generateCoordinates( float *X, float *Y, float *Z, float *M, info_t *info ) {
	double fix, fiy, fiz;

	srand( info->seed );
	for( int i = 0; i < info->n; i++ ) {
		fix = 2 * M_PI*rand( ) / (float) RAND_MAX;
		fiy = 2 * M_PI*rand( ) / (float) RAND_MAX;
		fiz = 2 * M_PI*rand( ) / (float) RAND_MAX;

		X[i] = (float) (cos( fiz ) * cos( fiy ) * info->sphereRadius);
		Y[i] = (float) (-sin( fiz ) * cos( fiy ) * info->sphereRadius);
		Z[i] = (float) (-sin( fiy ) * info->sphereRadius);
		M[i] = info->mass;
	}
}
void generateCoordinatesFloat4( float *coord, info_t *info ) {
	double fix, fiy, fiz;
	int index;

	srand( info->seed );
	for( int i = 0; i < info->n; i++ ) {
		fix = 2 * M_PI*rand( ) / (float) RAND_MAX;
		fiy = 2 * M_PI*rand( ) / (float) RAND_MAX;
		fiz = 2 * M_PI*rand( ) / (float) RAND_MAX;

		index = i * 4;
		coord[index] = (float) (cos( fiz ) * cos( fiy ) * info->sphereRadius);
		coord[index + 1] = (float)(-sin( fiz ) * cos( fiy ) * info->sphereRadius);
		coord[index + 2] = (float)(-sin( fiy ) * info->sphereRadius);
		coord[index + 3] = info->mass;
	}
}