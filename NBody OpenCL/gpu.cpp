#include <cstdio>
#include <cstdlib>
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/cl_gl.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Main.h"
#include "Timer.h"
#include "WOCL.h"

void gpu( info_t *info ) {
	printf( "\n\n== OpenCL ==                   N: %d, Steps: %d\n", info->n, info->steps );

	Timer t;
	WOCL gpu = WOCL( info->deviceType );
	gpu.SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, info->n ), 0 );

	// Host alokacija
	float *M = (float *) malloc( sizeof(float) * info->n );
	float *X = (float *) malloc( sizeof(float) * info->n );
	float *Y = (float *) malloc( sizeof(float) * info->n );
	float *Z = (float *) malloc( sizeof(float) * info->n );
	float *V = (float *) calloc( info->n, sizeof(float) );
	generateCoordinates( X, Y, Z, M, info );

	t.Tic();
	gpu.CreateAndBuildKernel( "res/kernelFloat1.cl", "kernelFloat1" );

	// Device alokacija in kopiranje podatkov
	cl_mem devX = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, X );
	cl_mem devY = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Y );
	cl_mem devZ = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Z );
	cl_mem devM = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, M );
	cl_mem devNewX = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE, NULL );
	cl_mem devNewY = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE, NULL );
	cl_mem devNewZ = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE, NULL );
	cl_mem devVX = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );
	cl_mem devVY = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );
	cl_mem devVZ = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );

	gpu.SetKernelArgument<cl_mem>( 6, &devVX );
	gpu.SetKernelArgument<cl_mem>( 7, &devVY );
	gpu.SetKernelArgument<cl_mem>( 8, &devVZ );
	gpu.SetKernelArgument<cl_mem>( 9, &devM );
	gpu.SetKernelArgument<cl_int>( 10, &(info->n) );
	gpu.SetKernelArgument<cl_float>( 11, &(info->eps) );
	gpu.SetKernelArgument<cl_float>( 12, &(info->kappa) );
	gpu.SetKernelArgument<cl_float>( 13, &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		gpu.SetKernelArgument<cl_mem>( 0, &devX );
		gpu.SetKernelArgument<cl_mem>( 1, &devY );
		gpu.SetKernelArgument<cl_mem>( 2, &devZ );
		gpu.SetKernelArgument<cl_mem>( 3, &devNewX );
		gpu.SetKernelArgument<cl_mem>( 4, &devNewY );
		gpu.SetKernelArgument<cl_mem>( 5, &devNewZ );

		gpu.ExecuteKernel();

		SWAP_MEM( devNewX, devX );
		SWAP_MEM( devNewY, devY );
		SWAP_MEM( devNewZ, devZ );
	}

	// Prenos rezultatov na gostitelja
	gpu.CopyDeviceToHost( &devX, X, info->n*sizeof(float) );
	gpu.CopyDeviceToHost( &devY, Y, info->n*sizeof(float) );
	gpu.CopyDeviceToHost( &devZ, Z, info->n*sizeof(float) );

	printf( "Cas izvajanja: %lf\n", t.Toc() );
	checkResults( X, Y, Z, info->n );

#pragma region Cleanup
	free( M );
	free( X );
	free( Y );
	free( Z );
#pragma endregion
}

void gpuVec( info_t *info ) {
	printf( "\n\n== OpenCL (float4) ==          N: %d, Steps: %d\n", info->n, info->steps );

	Timer t;
	WOCL gpu = WOCL( info->deviceType );
	gpu.SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, info->n ), 0 );

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V = (float *) calloc( info->n, 4 * sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	t.Tic( );
	gpu.CreateAndBuildKernel( "res/kernelVec.cl", "kernelVec" );

	// Device alokacija in kopiranje podatkov
	cl_mem devCoord		= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Coord );
	cl_mem devCoordNew	= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE,						 NULL );
	cl_mem devV			= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );

	gpu.SetKernelArgument<cl_mem>( 2, &devV );
	gpu.SetKernelArgument<cl_int>( 3, &(info->n) );
	gpu.SetKernelArgument<cl_float>( 4, &(info->eps) );
	gpu.SetKernelArgument<cl_float>( 5, &(info->kappa) );
	gpu.SetKernelArgument<cl_float>( 6, &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		gpu.SetKernelArgument<cl_mem>( 0, &devCoord );
		gpu.SetKernelArgument<cl_mem>( 1, &devCoordNew );
		gpu.ExecuteKernel( );
		SWAP_MEM( devCoord, devCoordNew);
	}

	// Prenos rezultatov na gostitelja
	gpu.CopyDeviceToHost( &devCoord, Coord, info->n*sizeof(cl_float4) );

	printf( "Cas izvajanja: %lf\n", t.Toc() );
	checkResultsFloat4( Coord, info->n );

	free( V );
	free( Coord );
}

void gpuVecLocal( info_t *info ) {
	printf( "\n\n== OpenCL (float4 local) ==    N: %d, Steps: %d\n", info->n, info->steps );

	Timer t;
	WOCL gpu = WOCL( info->deviceType );
	gpu.SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, info->n ), 0 );

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V = (float *) calloc( info->n, 4 * sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	t.Tic( );
	gpu.CreateAndBuildKernel( "res/kernelVecLocal.cl", "kernelVecLocal" );

	// Device alokacija in kopiranje podatkov
	cl_mem devCoord		= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Coord );
	cl_mem devCoordNew	= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE,						 NULL );
	cl_mem devV			= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );

	gpu.SetKernelArgument<cl_mem>( 2, &devV );
	gpu.SetKernelArgument<cl_int>( 3, &(info->n) );
	gpu.SetKernelArgument<cl_float>( 4, &(info->eps) );
	gpu.SetKernelArgument<cl_float>( 5, &(info->kappa) );
	gpu.SetKernelArgument<cl_float>( 6, &(info->dt) );
	gpu.SetAndAllocKernelArgument( 7, info->local_item_size * sizeof(cl_float4) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		gpu.SetKernelArgument<cl_mem>( 0, &devCoord );
		gpu.SetKernelArgument<cl_mem>( 1, &devCoordNew );
		gpu.ExecuteKernel( );
		SWAP_MEM( devCoord, devCoordNew );
	}

	// Prenos rezultatov na gostitelja
	gpu.CopyDeviceToHost( &devCoord, Coord, info->n*sizeof(cl_float4) );

	printf( "Cas izvajanja: %lf\n", t.Toc( ) );
	checkResultsFloat4( Coord, info->n );

	free( V );
	free( Coord );
}
