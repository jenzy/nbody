#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <mpi.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>

#include "Main.h"
#include "Timer.h"
#include "WOCL.h"

void mpi( info_t *info ) {
	int rank, numOfProcesses;
	Timer time;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &numOfProcesses );

	// counts in disps za Allgatherv, uposteva da eno telo = float4
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
		printf( "\n\n== MPI ==\n" ); 
		printf( "Processes: %d (N: %d, steps: %d)\n",numOfProcesses, info->n, info->steps ); fflush( stdout );

		generateCoordinatesFloat4( Coord, info );

		time.TicSimple();
	}

	// Send starting data
	MPI_Bcast( Coord, 4 * info->n, MPI_FLOAT, 0, MPI_COMM_WORLD );

	// Racunanje
	int m = disps[rank];
	int M = m + counts[rank];
	int n4 = 4 * info->n;
	float dt2 = 0.5f * info->dt*info->dt;
	for( int step = 0; step < info->steps; step++ ) {
		int vIndex = 0;
		for( int i = m; i < M; i += 4, vIndex += 3 ) { // za vsako telo m <= i < M
			ax = ay = az = 0.f;
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
			newCoord[i]		= myX + V[vIndex]     * info->dt + ax*dt2; // nov polozaj za telo
			newCoord[i + 1] = myY + V[vIndex + 1] * info->dt + ay*dt2;
			newCoord[i + 2] = myZ + V[vIndex + 2] * info->dt + az*dt2;
			newCoord[i + 3] = Coord[i + 3];

			V[vIndex]     += ax * info->dt; // nova hitrost za telo
			V[vIndex + 1] += ay * info->dt;
			V[vIndex + 2] += az * info->dt;
		}

		// Exchange data
		MPI_Allgatherv( &newCoord[m], counts[rank], MPI_FLOAT, Coord, counts, disps, MPI_FLOAT, MPI_COMM_WORLD );
	}
	
	if( rank == 0 ) {
		printf( "Time: %.3lf\n", time.TocSimple() );
		checkResultsFloat4( Coord, info->n );
	}
	
	// sprostimo rezerviran prostor
	free( Coord );
	free( newCoord );
	free( V );
	free( counts );
	free( disps );
}

void mpiOpenCL( info_t *info ) {

#pragma region MPI Init
	int rank, numOfProcesses;
	Timer time;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &numOfProcesses );

	if( rank == 0 ) {
		printf( "\n\n== MPI + OpenCL (float4 local) ==\n" );
		printf( "Processes: %d (N: %d, steps: %d)\n", numOfProcesses, info->n, info->steps ); fflush( stdout );
	}

	// counts in disps za Allgatherv, uposteva da eno telo = float4
	int* counts = (int*) malloc( numOfProcesses * sizeof(int) );
	int* disps = (int*) malloc( numOfProcesses * sizeof(int) );
	for( int i = 0; i < numOfProcesses; i++ ) {
		int m = (i*info->n) / numOfProcesses;
		int M = ((i + 1)*info->n) / numOfProcesses - 1;
		counts[i] = 4 * (M - m + 1);
		disps[i] = 4 * m;
	}

	int myN = counts[rank] / 4;
	int myStart = disps[rank] / 4;
#pragma endregion

	WOCL cl = WOCL( info->deviceType );
	cl.SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, myN ), 0 );

	// HOST alokacija ("float4", .w bo masa)
	float *Coord = (float *) malloc( 4 * sizeof(float) * info->n );
	float *newCoord = (float *) malloc( 4 * sizeof(float) * myN );
	float *V = (float *) calloc( 4 *sizeof(float), myN );
	if( rank == 0 )
		generateCoordinatesFloat4( Coord, info );
	MPI_Bcast( Coord, 4 * info->n, MPI_FLOAT, 0, MPI_COMM_WORLD );

	if( rank == 0 )
		time.TicSimple();

	// Device alokacija
	cl_mem devCoord		= cl.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE, NULL );
	cl_mem devCoordNew	= cl.CreateBuffer( myN*sizeof(cl_float4), CL_MEM_READ_WRITE, NULL );
	cl_mem devV			= cl.CreateBuffer( myN*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );
	
	// Priprava programa
	cl.CreateAndBuildKernel( "res/kernelCombo.cl", "kernelCombo" );

	cl.SetKernelArgument<cl_mem>( 0, &devCoord );
	cl.SetKernelArgument<cl_mem>( 1, &devCoordNew );
	cl.SetKernelArgument<cl_mem>( 2, &devV );
	cl.SetKernelArgument<cl_int>( 3, &myStart );
	cl.SetKernelArgument<cl_int>( 4, &(info->n) );
	cl.SetKernelArgument<cl_int>( 5, &myN );
	cl.SetKernelArgument<cl_float>( 6, &(info->eps) );
	cl.SetKernelArgument<cl_float>( 7, &(info->kappa) );
	cl.SetKernelArgument<cl_float>( 8, &(info->dt) );


	// zagon šèepca
	for( int step = 0; step < info->steps; step++ ) {
		cl.CopyHostToDevice( &devCoord, Coord, info->n*sizeof(cl_float4) );
		cl.ExecuteKernel();
		cl.CopyDeviceToHost( &devCoordNew, newCoord, myN * sizeof(cl_float4) );
		cl.Finish();
		MPI_Allgatherv( newCoord, counts[rank], MPI_FLOAT, Coord, counts, disps, MPI_FLOAT, MPI_COMM_WORLD );
	}

	if( rank == 0 ) {
		printf( "Time: %.3lf\n", time.TocSimple() );
		checkResultsFloat4( Coord, info->n );
	}

#pragma region Cleanup
	free( V );
	free( counts );
	free( disps );
	free( Coord );
#pragma endregion
}