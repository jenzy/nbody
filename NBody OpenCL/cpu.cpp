#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Main.h"
#include "Timer.h"

void cpu( info_t *info ) {
	printf( "\n\n== CPU ==\n" );
	printf( "N: %d, steps: %d\n", info->n, info->steps );
	int i, j, s;
	float dx, dy, dz, ax, ay, az, invr, force;
	Timer time;
	
	float *m = (float *) malloc( sizeof(float) * info->n );
	float *x = (float *) malloc( sizeof(float) * info->n );
	float *y = (float *) malloc( sizeof(float) * info->n );
	float *z = (float *) malloc( sizeof(float) * info->n );
	float *vx = (float *) calloc( sizeof(float), info->n );
	float *vy = (float *) calloc( sizeof(float), info->n );
	float *vz = (float *) calloc( sizeof(float), info->n );
	float *xnew = (float *) malloc( sizeof(float) * info->n );
	float *ynew = (float *) malloc( sizeof(float) * info->n );
	float *znew = (float *) malloc( sizeof(float) * info->n );

	generateCoordinates( x, y, z, m, info );	//inicializacija zacetnih polozajev in hitrosti

	time.Tic();
	float dt2 = 0.5f * info->dt * info->dt;
	for( s = 0; s < info->steps; s++ ) {
		for( i = 0; i < info->n; i++ ) { // za vsako telo i 
			ax = ay = az = 0.0;
			for( j = 0; j<info->n; j++ ) { // pregledamo vse ostale delce j 
				dx = x[j] - x[i];
				dy = y[j] - y[i];
				dz = z[j] - z[i];

				//Izvedemo trik brez uporabe if stavkov
				invr = 1.0f / sqrt( dx*dx + dy*dy + dz*dz + info->eps );
				force = info->kappa * m[j] * invr*invr*invr;

				ax += force*dx; // izracun skupnega pospeska
				ay += force*dy;
				az += force*dz;
			}
			xnew[i] = x[i] + vx[i] * info->dt + ax*dt2; // nov polozaj za telo i
			ynew[i] = y[i] + vy[i] * info->dt + ay*dt2;
			znew[i] = z[i] + vz[i] * info->dt + az*dt2;

			vx[i] += ax * info->dt; /* nova hitrost za telo i */
			vy[i] += ay * info->dt;
			vz[i] += az * info->dt;
		}
		SWAP( xnew, x );
		SWAP( ynew, y );
		SWAP( znew, z );
	}

	printf( "Cas izvajanja %lf\n", time.Toc() );
	checkResults( x, y, z, info->n );

#pragma region Cleanup
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
#pragma endregion
}

void cpuOpt( info_t *info ) {
	printf( "\n\n== CPU (Opt) ==\n" );
	printf( "N: %d, steps: %d\n", info->n, info->steps );
	int i, j, s;
	float dx, dy, dz, fdx, fdy, fdz, invr, force;
	Timer t;

	float *m = (float *) malloc( sizeof(float) * info->n );
	float *x = (float *) malloc( sizeof(float) * info->n );
	float *y = (float *) malloc( sizeof(float) * info->n );
	float *z = (float *) malloc( sizeof(float) * info->n );
	float *vx = (float *) calloc( sizeof(float), info->n );
	float *vy = (float *) calloc( sizeof(float), info->n );
	float *vz = (float *) calloc( sizeof(float), info->n );
	float *xnew = (float *) malloc( sizeof(float) * info->n );
	float *ynew = (float *) malloc( sizeof(float) * info->n );
	float *znew = (float *) malloc( sizeof(float) * info->n );
	float *ax = (float *) calloc( sizeof(float), info->n );
	float *ay = (float *) calloc( sizeof(float), info->n );
	float *az = (float *) calloc( sizeof(float), info->n );
	generateCoordinates( x, y, z, m, info );	//inicializacija zacetnih polozajev in hitrosti
	
	t.Tic();
	float dt2 = 0.5f * info->dt * info->dt;
	for( s = 0; s < info->steps; s++ ) {
		for( i = 0; i < info->n; i++ ) { 
			for( j = i+1; j<info->n; j++ ) { 
				dx = x[j] - x[i];
				dy = y[j] - y[i];
				dz = z[j] - z[i];

				invr = 1.0f / sqrt( dx*dx + dy*dy + dz*dz + info->eps );
				force = info->kappa*m[j] * invr*invr*invr;

				fdx = force*dx;
				fdy = force*dy;
				fdz = force*dz;

				ax[i] += fdx; 
				ay[i] += fdy;
				az[i] += fdz;

				ax[j] -= fdx;
				ay[j] -= fdy;
				az[j] -= fdz;
			}
		}

		for( i = 0; i < info->n; i++ ) {
			xnew[i] = x[i] + vx[i] * info->dt + ax[i] * dt2;
			ynew[i] = y[i] + vy[i] * info->dt + ay[i] * dt2;
			znew[i] = z[i] + vz[i] * info->dt + az[i] * dt2;

			vx[i] += ax[i] * info->dt;
			vy[i] += ay[i] * info->dt;
			vz[i] += az[i] * info->dt;

			ax[i] = ay[i] = az[i] = 0.f;
		}
		SWAP( xnew, x );
		SWAP( ynew, y );
		SWAP( znew, z );
	}
	
	printf( "Cas izvajanja %lf\n", t.Toc() );
	checkResults( x, y, z, info->n );

#pragma region Cleanup
	// sprostimo rezerviran prostor
	free( m );
	free( x );
	free( y );
	free( z );
	free( vx );
	free( vy );
	free( vz ); 
	free( ax );
	free( ay );
	free( az );
	free( xnew );
	free( ynew );
	free( znew );
#pragma endregion
}