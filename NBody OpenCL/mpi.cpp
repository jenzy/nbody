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

		// Generate coordinates and masses
		generateCoordinatesFloat4( Coord, info );

		time.Tic();
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
		for( int i = m; i < M; i += 4, vIndex += 3 ) { // za vsako telo
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
		printf( "Cas izvajanja %lf\n", time.Toc() );
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
	cl_event event;
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

#pragma region OpenCL Inicializacija
	cl_int	ret;
	
	// Platforma in naprava
	cl_platform_id platform_id = nullptr;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	cl_device_id device_id;
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	printf( "[%d] ", rank); PrintDeviceInfo( &platform_id, &device_id );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region OpenCL Delitev dela
	size_t local_item_size = info->local_item_size;
	size_t num_groups = ((myN - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "[%d] Delitev dela: local: %d | num_groups: %d | global: %d  (myStart:%d, myN: %d)\n", 
			rank, local_item_size, num_groups, global_item_size, myStart, myN );
#pragma endregion

	// HOST alokacija ("float4", .w bo masa)
	float *Coord = (float *) malloc( 4 * sizeof(float) * info->n );
	float *V = (float *) calloc( sizeof(float), 4 * myN );
	if( rank == 0 )
		generateCoordinatesFloat4( Coord, info );
	MPI_Bcast( Coord, 4 * info->n, MPI_FLOAT, 0, MPI_COMM_WORLD );

	if( rank == 0 )
		time.Tic();
	// Device alokacija
	cl_mem devCoord    = clCreateBuffer( context, CL_MEM_READ_WRITE,                        info->n*sizeof(cl_float4), NULL, &ret );
	cl_mem devCoordNew = clCreateBuffer( context, CL_MEM_READ_WRITE,                        info->n*sizeof(cl_float4), NULL, &ret );
	cl_mem devV        = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, myN*sizeof(cl_float4),     V,    &ret );
	
	// Priprava programa
	cl_program program;
	BuildKernel( &program, &context, &device_id, "kernelCombo.cl" );

	// priprava šcepca 
	cl_kernel krnl = clCreateKernel( program, "kernelCombo", &ret );
	ret |= clSetKernelArg( krnl, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( krnl, 3, sizeof(cl_int), (void *) &myStart );
	ret |= clSetKernelArg( krnl, 4, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( krnl, 5, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( krnl, 6, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( krnl, 7, sizeof(cl_float), (void *) &(info->dt) );


	// zagon šèepca
	for( int step = 0; step < info->steps; step++ ) {
		ret = clEnqueueWriteBuffer( command_queue, devCoord, CL_TRUE, 0, info->n*sizeof(cl_float4), Coord, 0, NULL, NULL );

		ret = clSetKernelArg( krnl, 0, sizeof(cl_mem), (void *) &devCoord );
		ret |= clSetKernelArg( krnl, 1, sizeof(cl_mem), (void *) &devCoordNew );
		ret |= clEnqueueNDRangeKernel( command_queue, krnl, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
		SWAP_MEM( devCoord, devCoordNew );

		// Prenos rezultatov na gostitelja in posiljanje ostalim
		ret = clEnqueueReadBuffer( command_queue, devCoord, CL_TRUE, 0, info->n*sizeof(cl_float4), Coord, 0, NULL, &event );
		clWaitForEvents( 1, &event );
		MPI_Allgatherv( MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, Coord, counts, disps, MPI_FLOAT, MPI_COMM_WORLD );
	}

	if( rank == 0 ) {
		printf( "Cas izvajanja %lf\n", time.Toc() );
		checkResultsFloat4( Coord, info->n );
	}

#pragma region Cleanup
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( krnl );
	ret = clReleaseProgram( program );
	ret = clReleaseMemObject( devV );
	ret = clReleaseMemObject( devCoord );
	ret = clReleaseMemObject( devCoordNew );
	ret = clReleaseCommandQueue( command_queue );
	ret = clReleaseContext( context );

	free( V );
	free( counts );
	free( disps );
	free( Coord );
#pragma endregion
}