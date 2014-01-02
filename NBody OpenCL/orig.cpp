#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "Main.h"


#define M_PI 3.1415926
#define SWAP(a,b) do {double *temp=a; a=b; b=temp;} while(0)

int cpuO( int argc, char *argv[] ) {

	int n = 1000; 				/* stevilo teles */
	int steps = 10; 			/* stevilo korakov */
	double sphereRadius = 10; 	/* nastavitve zacetne konfiguracije */
	double kappa = 1; 			/* gravitacijska konstanta */
	double mass = 1; 			/* masa teles */
	double eps = 0.0001; 		/* konstanta glajenja */
	double dt = 0.01; 			/* casovna konstanta 0.001*/

	int i, j, s;

	double fix, fiy, fiz;
	double dx, dy, dz, ax, ay, az, invr, force;
	double vscale = sqrt( kappa*mass*(n - 1) / (double) sphereRadius );  /* fizikalni zakoni */

	double *m = (double *) malloc( sizeof(double) *n );
	double *x = (double *) malloc( sizeof(double) *n );
	double *y = (double *) malloc( sizeof(double) *n );
	double *z = (double *) malloc( sizeof(double) *n );
	double *vx = (double *) malloc( sizeof(double) *n );
	double *vy = (double *) malloc( sizeof(double) *n );
	double *vz = (double *) malloc( sizeof(double) *n );
	double *xnew = (double *) malloc( sizeof(double) *n );
	double *ynew = (double *) malloc( sizeof(double) *n );
	double *znew = (double *) malloc( sizeof(double) *n );

	int *displacements, *dataCount;
	int min, max, blok, porocessNum;

	clock_t clockStart, clockEnd;


	/* inicializacija zacetnih polozajev in hitrosti */
	/* telesa so na plascu krogle z radijem sphereRadius */
	/* hitrost telesa lezi v ravnini, ki je pravokotna na radij */
	srand( time( NULL ) );

	for( i = 0; i<n; i++ ) {
		m[i] = mass;

		fix = 2 * M_PI*rand( ) / (double) RAND_MAX;
		fiy = 2 * M_PI*rand( ) / (double) RAND_MAX;
		fiz = 2 * M_PI*rand( ) / (double) RAND_MAX;

		x[i] = cos( fiz )*cos( fiy )*sphereRadius;
		y[i] = -sin( fiz )*cos( fiy )*sphereRadius;
		z[i] = -sin( fiy )*sphereRadius;
	}


	/* zacetno hitrost nastavimo na 0 */
	for( i = 0; i<n; i++ )
		vx[i] = vy[i] = vz[i] = 0.0;

	clockStart = clock( );
	for( s = 0; s < steps; s++ ) {

		for( i = 0; i < n; i++ ) { /* za vsako telo i */
			ax = ay = az = 0.0;

			for( j = 0; j<n; j++ ) { /* pregledamo vse ostale delce j */
				dx = x[j] - x[i];
				dy = y[j] - y[i];
				dz = z[j] - z[i];

				//Izvedemo trik brez uporabe if stavkov
				invr = 1.0 / sqrt( dx*dx + dy*dy + dz*dz + eps );
				force = kappa*m[j] * invr*invr*invr;

				ax += force*dx; /* izracun skupnega pospeska */
				ay += force*dy;
				az += force*dz;
			}

			xnew[i] = x[i] + vx[i] * dt + 0.5*ax*dt*dt; /* nov polozaj za telo i */
			ynew[i] = y[i] + vy[i] * dt + 0.5*ay*dt*dt;
			znew[i] = z[i] + vz[i] * dt + 0.5*az*dt*dt;

			printf( "%lf %lf\n", xnew[i], x[i] );

			vx[i] += ax*dt; /* nova hitrost za telo i */
			vy[i] += ay*dt;
			vz[i] += az*dt;
		}
		SWAP( xnew, x );
		SWAP( ynew, y );
		SWAP( znew, z );
		//checkResults( x, y, z, n );


	}


	clockEnd = clock( );

	printf( "Cas izvajanja %lf\n", (double) (clockEnd - clockStart) / CLOCKS_PER_SEC );

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

	return 0;
}
