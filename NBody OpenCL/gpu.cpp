#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <time.h>

#include "Main.h"
#include "Timer.h"

void gpu( info_t *info ) {
	cl_int	ret;
	Timer time;
	printf( "\n\n== OpenCL ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma in naprava
	cl_platform_id platform_id;
	cl_device_id device_id;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	PrintDeviceInfo( &platform_id, &device_id );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = info->local_item_size;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d, steps: %d)\n", local_item_size, num_groups, global_item_size, info->n, info->steps );
#pragma endregion

	// Host alokacija
	float *M = (float *) malloc( sizeof(float) * info->n );
	float *X = (float *) malloc( sizeof(float) * info->n );
	float *Y = (float *) malloc( sizeof(float) * info->n );
	float *Z = (float *) malloc( sizeof(float) * info->n );
	float *V = (float *) calloc( info->n, sizeof(float) );
	generateCoordinates( X, Y, Z, M, info );

	time.Tic();
	// Device alokacija in kopiranje podatkov
	cl_mem devX = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), X, &ret );
	cl_mem devY = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), Y, &ret );
	cl_mem devZ = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), Z, &ret );
	cl_mem devM = clCreateBuffer( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), M, &ret );
	cl_mem devNewX = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devNewY = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devNewZ = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devVX = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), V, &ret );
	cl_mem devVY = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), V, &ret );
	cl_mem devVZ = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(float), V, &ret );

	// Priprava programa
	cl_program program;
	BuildKernel( &program, &context, &device_id, "kernelFloat1.cl" );
	
	// priprava šcepca
	cl_kernel krnl = clCreateKernel( program, "kernelFloat1", &ret );
	ret |= clSetKernelArg( krnl, 6, sizeof(cl_mem), (void *) &devVX );
	ret |= clSetKernelArg( krnl, 7, sizeof(cl_mem), (void *) &devVY );
	ret |= clSetKernelArg( krnl, 8, sizeof(cl_mem), (void *) &devVZ );
	ret |= clSetKernelArg( krnl, 9, sizeof(cl_mem), (void *) &devM );
	ret |= clSetKernelArg( krnl, 10, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( krnl, 11, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( krnl, 12, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( krnl, 13, sizeof(cl_float), (void *) &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		ret = clSetKernelArg( krnl, 0, sizeof(cl_mem), (void *) &devX );
		ret |= clSetKernelArg( krnl, 1, sizeof(cl_mem), (void *) &devY );
		ret |= clSetKernelArg( krnl, 2, sizeof(cl_mem), (void *) &devZ );
		ret |= clSetKernelArg( krnl, 3, sizeof(cl_mem), (void *) &devNewX );
		ret |= clSetKernelArg( krnl, 4, sizeof(cl_mem), (void *) &devNewY );
		ret |= clSetKernelArg( krnl, 5, sizeof(cl_mem), (void *) &devNewZ );

		ret = clEnqueueNDRangeKernel( command_queue, krnl, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );

		SWAP_MEM( devNewX, devX );
		SWAP_MEM( devNewY, devY );
		SWAP_MEM( devNewZ, devZ );
	}

	// Prenos rezultatov na gostitelja
	ret = clEnqueueReadBuffer( command_queue, devX, CL_TRUE, 0, info->n*sizeof(float), X, 0, NULL, NULL );
	ret = clEnqueueReadBuffer( command_queue, devY, CL_TRUE, 0, info->n*sizeof(float), Y, 0, NULL, NULL );
	ret = clEnqueueReadBuffer( command_queue, devZ, CL_TRUE, 0, info->n*sizeof(float), Z, 0, NULL, NULL );

	printf( "Cas izvajanja: %lf\n", time.Toc() );
	checkResults( X, Y, Z, info->n );

#pragma region Cleanup
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( krnl );
	ret = clReleaseProgram( program );
	ret = clReleaseMemObject( devX );
	ret = clReleaseMemObject( devY );
	ret = clReleaseMemObject( devZ );
	ret = clReleaseMemObject( devM );
	ret = clReleaseMemObject( devNewX );
	ret = clReleaseMemObject( devNewY );
	ret = clReleaseMemObject( devNewZ );
	ret = clReleaseMemObject( devVX );
	ret = clReleaseMemObject( devVY );
	ret = clReleaseMemObject( devVZ );
	ret = clReleaseCommandQueue( command_queue );
	ret = clReleaseContext( context );

	free( M );
	free( X );
	free( Y );
	free( Z );
#pragma endregion
}

void gpuVec( info_t *info ) {
	cl_int	ret;
	Timer time;
	printf( "\n\n== OpenCL (float4) ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma in naprava
	cl_platform_id platform_id;
	cl_device_id device_id;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	PrintDeviceInfo( &platform_id, &device_id );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = info->local_item_size;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d, steps: %d)\n", local_item_size, num_groups, global_item_size, info->n, info->steps );
#pragma endregion

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V = (float *) calloc( info->n, 4*sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	time.Tic();
	// Device alokacija in kopiranje podatkov
	cl_mem devCoord    = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), Coord, &ret );
	cl_mem devCoordNew = clCreateBuffer( context, CL_MEM_READ_WRITE,						info->n*sizeof(cl_float4), NULL,  &ret );
	cl_mem devV        = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), V,     &ret );

	// Priprava programa
	cl_program program;
	BuildKernel( &program, &context, &device_id, "kernelVec.cl" );

	cl_kernel krnl = clCreateKernel( program, "kernelVec", &ret );
	ret = clSetKernelArg( krnl, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( krnl, 3, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( krnl, 4, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( krnl, 5, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( krnl, 6, sizeof(cl_float), (void *) &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		ret = clSetKernelArg( krnl, 0, sizeof(cl_mem), (void *) &devCoord );
		ret |= clSetKernelArg( krnl, 1, sizeof(cl_mem), (void *) &devCoordNew );
		ret |= clEnqueueNDRangeKernel( command_queue, krnl, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
		SWAP_MEM( devCoord, devCoordNew);
	}

	// Prenos rezultatov na gostitelja
	ret = clEnqueueReadBuffer( command_queue, devCoord, CL_TRUE, 0, info->n*sizeof(cl_float4), Coord, 0, NULL, NULL );

	printf( "Cas izvajanja: %lf\n", time.Toc() );
	checkResultsFloat4( Coord, info->n );

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
	free( Coord );
#pragma endregion
}

void gpuVecLocal( info_t *info ) {
	cl_int	ret;
	Timer time;
	printf( "\n\n== OpenCL (float4 local) ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma in naprava
	cl_platform_id platform_id;
	cl_device_id device_id;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	PrintDeviceInfo( &platform_id, &device_id );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = info->local_item_size;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d, steps: %d)\n", local_item_size, num_groups, global_item_size, info->n, info->steps );
#pragma endregion

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V =    (float *) calloc( info->n, 4 * sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	time.Tic();
	// Device alokacija in kopiranje podatkov
	cl_mem devCoord    = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), Coord, &ret );
	cl_mem devCoordNew = clCreateBuffer( context, CL_MEM_READ_WRITE,                        info->n*sizeof(cl_float4), NULL,  &ret );
	cl_mem devV        = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), V,     &ret );

	// Priprava programa
	cl_program program;
	BuildKernel( &program, &context, &device_id, "kernelVecLocal.cl" );
	
	// priprava šcepca 
	cl_kernel krnl = clCreateKernel( program, "kernelVecLocal", &ret );
	ret |= clSetKernelArg( krnl, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( krnl, 3, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( krnl, 4, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( krnl, 5, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( krnl, 6, sizeof(cl_float), (void *) &(info->dt) );
	ret |= clSetKernelArg( krnl, 7, info->local_item_size * sizeof(cl_float4), NULL );

	// zagon šèepca
	for( int i = 0; i < info->steps; i++ ) {
		ret = clSetKernelArg( krnl, 0, sizeof(cl_mem), (void *) &devCoord );
		ret |= clSetKernelArg( krnl, 1, sizeof(cl_mem), (void *) &devCoordNew );
		ret |= clEnqueueNDRangeKernel( command_queue, krnl, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
		SWAP_MEM( devCoord, devCoordNew );
	}

	// Prenos rezultatov na gostitelja
	ret = clEnqueueReadBuffer( command_queue, devCoord, CL_TRUE, 0, info->n*sizeof(cl_float4), Coord, 0, NULL, NULL );

	printf( "Cas izvajanja %lf\n", time.Toc());
	checkResultsFloat4( Coord, info->n );

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
	free( Coord );
#pragma endregion
}
