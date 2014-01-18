#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <mpi.h>

#include "Main.h"

void mpi( info_t *info ) {
	int rank, numOfProcesses;
	clock_t clockStart, clockEnd;

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &numOfProcesses );

	int* counts = (int*) malloc( numOfProcesses * sizeof(int) );
	int* disps = (int*) malloc( numOfProcesses * sizeof(int) );
	for( int i = 0; i<numOfProcesses; i++ ) {
		int m = (i*info->n) / numOfProcesses;
		int M = ((i + 1)*info->n) / numOfProcesses - 1;
		counts[i] = 4 * (M - m + 1);
		disps[i] = 4 * m;
	}

	// Alokacija ("float4", .w bo masa)
	float *Coord = (float *) malloc( 4 * sizeof(float) * info->n );
	float *newCoord = (float *) malloc( 4 * sizeof(float) * info->n );
	float *V = (float *) calloc( sizeof(float), 3 * counts[rank] );
	float ax, ay, az, dx, dy, dz, myX, myY, myZ, invr, force;

	
	if( rank == 0 ) {
		printf( "\n\n== MPI ==\n" ); fflush( stdout );

		generateCoordinatesFloat4( Coord, info );

		clockStart = clock( );
	}

	// Send starting data
	MPI_Bcast( Coord, 4 * info->n, MPI_FLOAT, 0, MPI_COMM_WORLD );

	//Racunanje
	int M = disps[rank] + counts[rank];
	int n4 = 4 * info->n;
	float dt2 = 0.5f * info->dt*info->dt;
	for( int step = 0; step < info->steps; step++ ) {
		int vIndex = 0;
		for( int i = disps[rank]; i < M; i += 4, vIndex += 3 ) { // za vsako telo
			ax = ay = az = 0.0;
			myX = Coord[i]; myY = Coord[i + 1]; myZ = Coord[i + 2];
			for( int j = 0; j < n4; j += 4 ) { // pregledamo vse ostale delce
				dx = Coord[j]     - myX;
				dy = Coord[j + 1] - myY;
				dz = Coord[j + 2] - myZ;

				invr = 1.0f / sqrt( dx*dx + dy*dy + dz*dz + info->eps );
				force = info->kappa * Coord[j + 3] * invr*invr*invr;

				ax += force*dx;
				ay += force*dy;
				az += force*dz;
			}
			newCoord[i] = myX + V[vIndex] * info->dt + ax*dt2; // nov polozaj za telo
			newCoord[i + 1] = myY + V[vIndex + 1] * info->dt + ay*dt2;
			newCoord[i + 2] = myZ + V[vIndex + 2] * info->dt + az*dt2;
			newCoord[i + 3] = Coord[i + 3];

			V[vIndex] += ax * info->dt; // nova hitrost za telo
			V[vIndex + 1] += ay * info->dt;
			V[vIndex + 2] += az * info->dt;
		}

		// Exchange data
		MPI_Allgatherv( &newCoord[disps[rank]], counts[rank], MPI_FLOAT, Coord, counts, disps, MPI_FLOAT, MPI_COMM_WORLD );
	}
	
	
	if( rank == 0 ) {
		clockEnd = clock( );
		printf( "Cas izvajanja %lf\n", (double) (clockEnd - clockStart) / CLOCKS_PER_SEC );
		checkResultsFloat4( Coord, info->n );
	}
	
	// sprostimo rezerviran prostor
	free( Coord );
	free( newCoord );
	free( V );
}