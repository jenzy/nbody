#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <time.h>

#include "Main.h"

#define SWAP_MEM(a,b) do {cl_mem temp=a; a=b; b=temp;} while(0)

void gpuSyncInKernelTest( info_t *info ) {
	cl_int	ret;
	char *buf = nullptr;
	clock_t clockStart, clockEnd;
	cl_event event;

	printf( "\n\n== OpenCL (Sync in Kernel Test) ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma
	cl_platform_id platform_id = nullptr;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	buf = GetPlatformName( &platform_id );
	printf( "Platforma: %s\n", buf );
	free( buf );

	// Naprava 
	cl_device_id device_id;
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	buf = GetDeviceName( &device_id );
	printf( "Naprava: %s\n", buf );
	free( buf );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = 256;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d)\n", local_item_size, num_groups, global_item_size, info->n );
#pragma endregion

	// Host alokacija
	float *M = (float *) malloc( sizeof(float) * info->n );
	float *X = (float *) malloc( sizeof(float) * info->n );
	float *Y = (float *) malloc( sizeof(float) * info->n );
	float *Z = (float *) malloc( sizeof(float) * info->n );
	generateCoordinates( X, Y, Z, info );
	for( int i = 0; i < info->n; i++ )
		M[i] = info->mass;
	float *V = (float *) calloc( info->n, sizeof(float) );

	clockStart = clock( );
	// Device alokacija
	cl_mem devX = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devY = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devZ = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devM = clCreateBuffer( context, CL_MEM_READ_ONLY, info->n*sizeof(float), NULL, &ret );
	cl_mem devNewX = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devNewY = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devNewZ = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devVX = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devVY = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );
	cl_mem devVZ = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(float), NULL, &ret );

	// Kopiranje podatkov
	ret = clEnqueueWriteBuffer( command_queue, devX, CL_TRUE, 0, info->n*sizeof(float), X, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, devY, CL_TRUE, 0, info->n*sizeof(float), Y, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, devZ, CL_TRUE, 0, info->n*sizeof(float), Z, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, devM, CL_TRUE, 0, info->n*sizeof(float), M, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, devVX, CL_TRUE, 0, info->n*sizeof(float), V, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, devVY, CL_TRUE, 0, info->n*sizeof(float), V, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, devVZ, CL_TRUE, 0, info->n*sizeof(float), V, 0, NULL, NULL );


	// Priprava programa
	char *source_str = ReadKernelFromFile( "kernelFloat1.cl", NULL );
	cl_program program = clCreateProgramWithSource( context, 1, (const char **) &source_str, NULL, &ret );
	ret = clBuildProgram( program, 1, &device_id, NULL, NULL, NULL );	// Prevajanje
	if( ret != CL_SUCCESS ) {
		PrintBuildLog( &program, &device_id );
		exit( 1 );
	}

	// priprava šcepca EVEN
	cl_kernel kernelEven = clCreateKernel( program, "kernelSyncTest", &ret );
	ret = clSetKernelArg( kernelEven, 0, sizeof(cl_mem), (void *) &devX );
	ret |= clSetKernelArg( kernelEven, 1, sizeof(cl_mem), (void *) &devY );
	ret |= clSetKernelArg( kernelEven, 2, sizeof(cl_mem), (void *) &devZ );
	ret |= clSetKernelArg( kernelEven, 3, sizeof(cl_mem), (void *) &devNewX );
	ret |= clSetKernelArg( kernelEven, 4, sizeof(cl_mem), (void *) &devNewY );
	ret |= clSetKernelArg( kernelEven, 5, sizeof(cl_mem), (void *) &devNewZ );
	ret |= clSetKernelArg( kernelEven, 6, sizeof(cl_mem), (void *) &devVX );
	ret |= clSetKernelArg( kernelEven, 7, sizeof(cl_mem), (void *) &devVY );
	ret |= clSetKernelArg( kernelEven, 8, sizeof(cl_mem), (void *) &devVZ );
	ret |= clSetKernelArg( kernelEven, 9, sizeof(cl_mem), (void *) &devM );
	ret |= clSetKernelArg( kernelEven, 10, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( kernelEven, 11, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( kernelEven, 12, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( kernelEven, 13, sizeof(cl_float), (void *) &(info->dt) );
	ret |= clSetKernelArg( kernelEven, 14, sizeof(cl_float), (void *) &(info->steps) );


	// šcepec: zagon
	ret = clEnqueueNDRangeKernel( command_queue, kernelEven, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
	
	// Prenos rezultatov na gostitelja
	float *newX = (float*) malloc( info->n*sizeof(float) );
	float *newY = (float*) malloc( info->n*sizeof(float) );
	float *newZ = (float*) malloc( info->n*sizeof(float) );
	ret = clEnqueueReadBuffer( command_queue, devX, CL_TRUE, 0, info->n*sizeof(float), newX, 0, NULL, NULL );
	ret = clEnqueueReadBuffer( command_queue, devY, CL_TRUE, 0, info->n*sizeof(float), newY, 0, NULL, NULL );
	ret = clEnqueueReadBuffer( command_queue, devZ, CL_TRUE, 0, info->n*sizeof(float), newZ, 0, NULL, NULL );
	clockEnd = clock( );


	//checkResults( X, Y, Z, info->n );
	printf( "Cas izvajanja %lf\n", (double) (clockEnd - clockStart) / CLOCKS_PER_SEC );
	checkResults( newX, newY, newZ, info->n );

#pragma region Cleanup
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( kernelEven );
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
	free( newX );
	free( newY );
	free( newZ );
	free( source_str );
#pragma endregion
}

void gpu( info_t *info ) {
	cl_int	ret;
	char *buf = nullptr;
	clock_t clockStart, clockEnd;
	cl_event event;

	printf( "\n\n== OpenCL ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma
	cl_platform_id platform_id = nullptr;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	buf = GetPlatformName( &platform_id );
	printf( "Platforma: %s\n", buf );
	free( buf );
	
	// Naprava 
	cl_device_id device_id;
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	buf = GetDeviceName( &device_id );
	printf( "Naprava: %s\n", buf );
	free( buf );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = 256;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d)\n", local_item_size, num_groups, global_item_size, info->n );
#pragma endregion

	// Host alokacija
	float *M = (float *) malloc( sizeof(float) * info->n );
	float *X = (float *) malloc( sizeof(float) * info->n );
	float *Y = (float *) malloc( sizeof(float) * info->n );
	float *Z = (float *) malloc( sizeof(float) * info->n );
	generateCoordinates( X, Y, Z, info );
	for( int i = 0; i < info->n; i++ )
		M[i] = info->mass;
	float *V = (float *) calloc( info->n, sizeof(float) );

	clockStart = clock( );
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
	char *source_str = ReadKernelFromFile( "kernelFloat1.cl", NULL );
	cl_program program = clCreateProgramWithSource( context, 1, (const char **) &source_str, NULL, &ret );
	ret = clBuildProgram( program, 1, &device_id, NULL, NULL, NULL );	// Prevajanje
	if( ret != CL_SUCCESS ) {
		PrintBuildLog( &program, &device_id );
		exit( 1 );
	}

	// priprava šcepca EVEN
	cl_kernel kernelEven = clCreateKernel( program, "kernelFloat1", &ret );
	ret = clSetKernelArg( kernelEven, 0, sizeof(cl_mem), (void *) &devX );
	ret |= clSetKernelArg( kernelEven, 1, sizeof(cl_mem), (void *) &devY );
	ret |= clSetKernelArg( kernelEven, 2, sizeof(cl_mem), (void *) &devZ );
	ret |= clSetKernelArg( kernelEven, 3, sizeof(cl_mem), (void *) &devNewX );
	ret |= clSetKernelArg( kernelEven, 4, sizeof(cl_mem), (void *) &devNewY );
	ret |= clSetKernelArg( kernelEven, 5, sizeof(cl_mem), (void *) &devNewZ );
	ret |= clSetKernelArg( kernelEven, 6, sizeof(cl_mem), (void *) &devVX );
	ret |= clSetKernelArg( kernelEven, 7, sizeof(cl_mem), (void *) &devVY );
	ret |= clSetKernelArg( kernelEven, 8, sizeof(cl_mem), (void *) &devVZ );
	ret |= clSetKernelArg( kernelEven, 9, sizeof(cl_mem), (void *) &devM );
	ret |= clSetKernelArg( kernelEven, 10, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( kernelEven, 11, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( kernelEven, 12, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( kernelEven, 13, sizeof(cl_float), (void *) &(info->dt) );

	// priprava šcepca ODD
	cl_kernel kernelOdd = clCreateKernel( program, "kernelFloat1", &ret );
	ret = clSetKernelArg( kernelOdd, 0, sizeof(cl_mem), (void *) &devNewX );
	ret |= clSetKernelArg( kernelOdd, 1, sizeof(cl_mem), (void *) &devNewY );
	ret |= clSetKernelArg( kernelOdd, 2, sizeof(cl_mem), (void *) &devNewZ );
	ret |= clSetKernelArg( kernelOdd, 3, sizeof(cl_mem), (void *) &devX );
	ret |= clSetKernelArg( kernelOdd, 4, sizeof(cl_mem), (void *) &devY );
	ret |= clSetKernelArg( kernelOdd, 5, sizeof(cl_mem), (void *) &devZ );
	ret |= clSetKernelArg( kernelOdd, 6, sizeof(cl_mem), (void *) &devVX );
	ret |= clSetKernelArg( kernelOdd, 7, sizeof(cl_mem), (void *) &devVY );
	ret |= clSetKernelArg( kernelOdd, 8, sizeof(cl_mem), (void *) &devVZ );
	ret |= clSetKernelArg( kernelOdd, 9, sizeof(cl_mem), (void *) &devM );
	ret |= clSetKernelArg( kernelOdd, 10, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( kernelOdd, 11, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( kernelOdd, 12, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( kernelOdd, 13, sizeof(cl_float), (void *) &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		if( i%2 == 0 )
			ret = clEnqueueNDRangeKernel( command_queue, kernelEven, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
		else
			ret = clEnqueueNDRangeKernel( command_queue, kernelOdd, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
	}

	if( info->steps % 2 != 0 ) {
		SWAP_MEM( devNewX, devX );
		SWAP_MEM( devNewY, devY );
		SWAP_MEM( devNewZ, devZ );
	}

	// Prenos rezultatov na gostitelja
	float *newX = (float*) malloc( info->n*sizeof(float) );
	float *newY = (float*) malloc( info->n*sizeof(float) );
	float *newZ = (float*) malloc( info->n*sizeof(float) );
	ret = clEnqueueReadBuffer( command_queue, devX, CL_TRUE, 0, info->n*sizeof(float), newX, 0, NULL, NULL );
	ret = clEnqueueReadBuffer( command_queue, devY, CL_TRUE, 0, info->n*sizeof(float), newY, 0, NULL, NULL );
	ret = clEnqueueReadBuffer( command_queue, devZ, CL_TRUE, 0, info->n*sizeof(float), newZ, 0, NULL, NULL );
	clockEnd = clock( );

	printf( "Cas izvajanja %lf\n", (double) (clockEnd - clockStart) / CLOCKS_PER_SEC );
	checkResults( newX, newY, newZ, info->n );

#pragma region Cleanup
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( kernelEven );
	ret = clReleaseKernel( kernelOdd );
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
	free( newX );
	free( newY );
	free( newZ );
	free( source_str );
#pragma endregion
}

void gpuVec( info_t *info ) {
	cl_int	ret;
	char *buf = nullptr;
	clock_t clockStart, clockEnd;
	printf( "\n\n== OpenCL (float4) ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma
	cl_platform_id platform_id = nullptr;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	buf = GetPlatformName( &platform_id );
	printf( "Platforma: %s\n", buf );
	free( buf );

	// Naprava 
	cl_device_id device_id;
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	buf = GetDeviceName( &device_id );
	printf( "Naprava: %s\n", buf );
	free( buf );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = 256;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d)\n", local_item_size, num_groups, global_item_size, info->n );
#pragma endregion

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V = (float *) calloc( info->n, 4*sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	clockStart = clock( );
	// Device alokacija in kopiranje podatkov
	cl_mem devCoord = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), Coord, &ret );
	cl_mem devCoordNew = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(cl_float4), NULL, &ret );
	cl_mem devV = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), V, &ret );

#pragma region Prevajanje kernela
	// Priprava programa
	char *source_str = ReadKernelFromFile( "kernelVec.cl", NULL );
	cl_program program = clCreateProgramWithSource( context, 1, (const char **) &source_str, NULL, &ret );
	ret = clBuildProgram( program, 1, &device_id, NULL, NULL, NULL );	// Prevajanje
	if( ret != CL_SUCCESS ) {
		PrintBuildLog( &program, &device_id );
		exit( 1 );
	}
#pragma endregion

	// priprava šcepca EVEN
	cl_kernel kernelEven = clCreateKernel( program, "kernelVec", &ret );
	ret = clSetKernelArg( kernelEven, 0, sizeof(cl_mem), (void *) &devCoord );
	ret |= clSetKernelArg( kernelEven, 1, sizeof(cl_mem), (void *) &devCoordNew );
	ret |= clSetKernelArg( kernelEven, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( kernelEven, 3, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( kernelEven, 4, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( kernelEven, 5, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( kernelEven, 6, sizeof(cl_float), (void *) &(info->dt) );

	// priprava šcepca ODD
	cl_kernel kernelOdd = clCreateKernel( program, "kernelVec", &ret );
	ret |= clSetKernelArg( kernelOdd, 0, sizeof(cl_mem), (void *) &devCoordNew );
	ret |= clSetKernelArg( kernelOdd, 1, sizeof(cl_mem), (void *) &devCoord );
	ret |= clSetKernelArg( kernelOdd, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( kernelOdd, 3, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( kernelOdd, 4, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( kernelOdd, 5, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( kernelOdd, 6, sizeof(cl_float), (void *) &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		if( i % 2 == 0 )
			ret = clEnqueueNDRangeKernel( command_queue, kernelEven, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
		else
			ret = clEnqueueNDRangeKernel( command_queue, kernelOdd, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
	}
	if( info->steps % 2 != 0 ) 
		SWAP_MEM( devCoord, devCoordNew);

	// Prenos rezultatov na gostitelja
	ret = clEnqueueReadBuffer( command_queue, devCoord, CL_TRUE, 0, info->n*sizeof(cl_float4), Coord, 0, NULL, NULL );
	clockEnd = clock( );

	printf( "Cas izvajanja %lf\n", (double) (clockEnd - clockStart) / CLOCKS_PER_SEC );
	checkResultsFloat4( Coord, info->n );

#pragma region Cleanup
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( kernelEven );
	ret = clReleaseKernel( kernelOdd );
	ret = clReleaseProgram( program );
	ret = clReleaseMemObject( devV );
	ret = clReleaseMemObject( devCoord );
	ret = clReleaseMemObject( devCoordNew );
	ret = clReleaseCommandQueue( command_queue );
	ret = clReleaseContext( context );

	free( V );
	free( Coord );
	free( source_str );
#pragma endregion
}