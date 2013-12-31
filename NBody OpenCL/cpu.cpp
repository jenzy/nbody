#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "Main.h"

void cpuOriginal( info_t *info ) {
	int i, j, s;
	clock_t clockStart, clockEnd;

	double dx, dy, dz, ax, ay, az, invr, force;
	//double vscale = sqrt( kappa*mass*(n - 1) / (double) sphereRadius );  /* fizikalni zakoni */
	//int *displacements, *dataCount;
	//int min, max, blok, porocessNum;

	double *m = (double *) malloc( sizeof(double) * info->n );
	double *x = (double *) malloc( sizeof(double) * info->n );
	double *y = (double *) malloc( sizeof(double) * info->n );
	double *z = (double *) malloc( sizeof(double) * info->n );
	double *vx = (double *) malloc( sizeof(double) * info->n );
	double *vy = (double *) malloc( sizeof(double) * info->n );
	double *vz = (double *) malloc( sizeof(double) * info->n );
	double *xnew = (double *) malloc( sizeof(double) * info->n );
	double *ynew = (double *) malloc( sizeof(double) * info->n );
	double *znew = (double *) malloc( sizeof(double) * info->n );

	generateCoordinates( x, y, z, info );	//inicializacija zacetnih polozajev in hitrosti

	for( i = 0; i < info->n; i++ ) {
		vx[i] = vy[i] = vz[i] = 0.0;	//zacetno hitrost nastavimo na 0
		m[i] = info->mass;
	}

	clockStart = clock( );
	for( s = 0; s < info->steps; s++ ) {
		for( i = 0; i < info->n; i++ ) { // za vsako telo i 
			ax = ay = az = 0.0;
			for( j = 0; j<info->n; j++ ) { // pregledamo vse ostale delce j 
				dx = x[j] - x[i];
				dy = y[j] - y[i];
				dz = z[j] - z[i];

				//Izvedemo trik brez uporabe if stavkov
				invr = 1.0 / sqrt( dx*dx + dy*dy + dz*dz + info->eps );
				force = info->kappa*m[j] * invr*invr*invr;

				ax += force*dx; // izracun skupnega pospeska
				ay += force*dy;
				az += force*dz;
			}
			double dt2 = info->dt*info->dt;
			xnew[i] = x[i] + vx[i] * info->dt + 0.5*ax*dt2; // nov polozaj za telo i
			ynew[i] = y[i] + vy[i] * info->dt + 0.5*ay*dt2;
			znew[i] = z[i] + vz[i] * info->dt + 0.5*az*dt2;

			vx[i] += ax * info->dt; /* nova hitrost za telo i */
			vy[i] += ay * info->dt;
			vz[i] += az * info->dt;
		}
		SWAP( xnew, x );
		SWAP( ynew, y );
		SWAP( znew, z );
	}

	clockEnd = clock( );

	printf( "Cas izvajanja %lf\n", (double) (clockEnd - clockStart) / CLOCKS_PER_SEC );
	checkResults( x, y, z, info->n );

	// sprostimo rezerviran prostor
	free( m );
	free( x );
	free( y );
	free( z );
	free( vx );
	free( vy );
	free( vz );
	free( xnew );
	free( ynew );
	free( znew );
}
