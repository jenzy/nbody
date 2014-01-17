#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
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
/*
void gpu( void ) {
	int i;
	cl_int ret;

	int vectorSize = SIZE;

	// Inicializacija vektorjev
	double *A = (double*) malloc( vectorSize*sizeof(double) );
	double *B = (double*) malloc( vectorSize*sizeof(double) );
	for( i = 0; i < vectorSize; i++ ) {
		A[i] = i;
		B[i] = vectorSize - i;
	}
	// Podatki o platformi
	cl_platform_id	platform_id;
	cl_uint			ret_num_platforms;

	ret = clGetPlatformIDs( 1, &platform_id, &ret_num_platforms );

	// Naprava 
	cl_device_id	device_id[2];
	cl_uint			ret_num_devices;
	size_t buf_len;
	char *buf;
	ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 2,// izbrana platforma, naprava (najprej GPU, "ce je ni CPU), koliko naprav nas
						  device_id, &ret_num_devices );				// zanima, kazalec na naprave, dejansko "stevilo platform
	for( i = 0; i<ret_num_devices; i++ ) {
		ret = clGetDeviceInfo( device_id[i], CL_DEVICE_NAME, 0, NULL, &buf_len );
		buf = (char *) malloc( sizeof(char) *(buf_len + 1) );
		buf[buf_len] = '\0';
		ret = clGetDeviceInfo( device_id[i], CL_DEVICE_NAME, buf_len, buf, NULL );
		printf( "naprava: %s\n", buf );
		free( buf );
	}

	// Kontekst
	cl_context context = clCreateContext( NULL, 1, &device_id[1], NULL, NULL, &ret );

	// Ukazna vrsta
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id[1], 0, &ret );

	// Delitev dela
	size_t local_item_size = 64;
	size_t num_groups = ((vectorSize - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

	// Alokacija pomnilnika
	cl_mem a_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
									   vectorSize*sizeof(double), NULL, &ret );
	cl_mem b_mem_obj = clCreateBuffer( context, CL_MEM_READ_ONLY,
									   vectorSize*sizeof(double), NULL, &ret );
	cl_mem c_mem_obj = clCreateBuffer( context, CL_MEM_WRITE_ONLY,
									   vectorSize*sizeof(double), NULL, &ret );

	// Kopiranje podatkov
	ret = clEnqueueWriteBuffer( command_queue, a_mem_obj, CL_TRUE, 0,
								vectorSize*sizeof(double), A, 0, NULL, NULL );
	ret = clEnqueueWriteBuffer( command_queue, b_mem_obj, CL_TRUE, 0,
								vectorSize*sizeof(double), B, 0, NULL, NULL );

	// Priprava programa
	char *source_str = ReadKernelFromFile( "kernel.cl", NULL );
	cl_program program = clCreateProgramWithSource( context, 1, (const char **) &source_str,
													NULL, &ret );

	// Prevajanje
	ret = clBuildProgram( program, 1, &device_id[1], NULL, NULL, NULL );

	
	// kazalec na funkcijo, uporabni"ski argumenti
	size_t build_log_len;
	char *log, ch;
	ret = clGetProgramBuildInfo( program, device_id[1], CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len );
	log = (char *) malloc( sizeof(char) *(build_log_len + 1) );
	ret = clGetProgramBuildInfo( program, device_id[1], CL_PROGRAM_BUILD_LOG, build_log_len, log, NULL );
	printf( "Build log: %s\n", log );
	free( log );
	getchar( );

	// "s"cepca: priprava objekta
	cl_kernel kernel = clCreateKernel( program, "vector_add", &ret );

	// "s"cepca: argumenti
	ret = clSetKernelArg( kernel, 0, sizeof(cl_mem), (void *) &a_mem_obj );
	ret |= clSetKernelArg( kernel, 1, sizeof(cl_mem), (void *) &b_mem_obj );
	ret |= clSetKernelArg( kernel, 2, sizeof(cl_mem), (void *) &c_mem_obj );
	ret |= clSetKernelArg( kernel, 3, sizeof(cl_int), (void *) &vectorSize );

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel( command_queue, kernel, 1, NULL,
								  &global_item_size, &local_item_size, 0, NULL, NULL );


	// Prenos rezultatov na gostitelja
	double *C = (double*) malloc( vectorSize*sizeof(double) );
	ret = clEnqueueReadBuffer( command_queue, c_mem_obj, CL_TRUE, 0,
							   vectorSize*sizeof(double), C, 0, NULL, NULL );

	// Prikaz rezultatov
	for( i = 0; i < vectorSize; i++ )
		printf( "%lf + %lf = %lf\n", A[i], B[i], C[i] );

	// "ci"s"cenje
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( kernel );
	ret = clReleaseProgram( program );
	ret = clReleaseMemObject( a_mem_obj );
	ret = clReleaseMemObject( b_mem_obj );
	ret = clReleaseMemObject( c_mem_obj );
	ret = clReleaseCommandQueue( command_queue );
	ret = clReleaseContext( context );

	free( A );
	free( B );
	free( C );
}
*/