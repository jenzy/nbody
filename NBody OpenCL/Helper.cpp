#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "Main.h"

#define CHECK_N 5


/* inicializacija zacetnih polozajev in hitrosti */
/* telesa so na plascu krogle z radijem sphereRadius */
/* hitrost telesa lezi v ravnini, ki je pravokotna na radij */
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

void generateCoordinatesSphereFloat4( float *coord, info_t *info ) {
	srand( info->seed );

	int index;
	float x, y, z, phi, theta, rcostheta;
	float r = (float)info->sphereRadius;
	float r2 = 2 * r;
	float pi = (float)M_PI;
	float pi2 = 2 * pi;

	for( int i = 0; i < info->n; i++ ) {
		z = r2 * rand_0_1() - r;
		phi = pi2 * rand_0_1();
		theta = asin( z / r );
		rcostheta = r * cos( theta );
		x = rcostheta * cos( phi );
		y = rcostheta * sin( phi );

		index = i * 4;
		coord[index]	 = x;
		coord[index + 1] = y;
		coord[index + 2] = z;
		coord[index + 3] = info->mass;
	}
}
