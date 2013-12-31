#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE	8192
#define SIZE			4

const char *source_str =
"__kernel void vector_add(__global const double *A,		\n"
"						  __global const double *B,		\n"
"						  __global double *C,				\n"
"						  int size)						\n"
"{														\n"
"	// globalni indeks elementa							\n"
"	int i = get_global_id(0);							\n"
"	// izracun											\n"
"	while( i < size )									\n"
"	{													\n"
"		C[i] = A[i] + B[i];								\n"
"		i += get_global_size(0);						\n"
"	}													\n"
"}														\n";

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

	// Podatki o napravi
	/*cl_device_id	device_id;
	cl_uint			ret_num_devices;

	ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_DEFAULT, 1,
						  &device_id, &ret_num_devices );*/

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
