#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>

#include <CL/cl.h>

void clinfo( ) {
	int i, j;
	char* value;
	size_t valueSize;
	cl_uint platformCount;
	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_uint maxComputeUnits;

	// get all platforms
	clGetPlatformIDs( 0, NULL, &platformCount );
	platforms = (cl_platform_id*) malloc( sizeof(cl_platform_id) * platformCount );
	clGetPlatformIDs( platformCount, platforms, NULL );

	for( i = 0; i < platformCount; i++ ) {

		// get all devices
		clGetDeviceIDs( platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount );
		printf( "Device count: %d\n", deviceCount );
		devices = (cl_device_id*) malloc( sizeof(cl_device_id) * deviceCount );
		clGetDeviceIDs( platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL );

		// for each device print critical attributes
		for( j = 0; j < deviceCount; j++ ) {

			// print device name
			clGetDeviceInfo( devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize );
			value = (char*) malloc( valueSize );
			clGetDeviceInfo( devices[j], CL_DEVICE_NAME, valueSize, value, NULL );
			printf( "%d. Device: %s\n", j + 1, value );
			free( value );

			// print vendor
			clGetDeviceInfo( devices[j], CL_DEVICE_VENDOR, 0, NULL, &valueSize );
			value = (char*) malloc( valueSize );
			clGetDeviceInfo( devices[j], CL_DEVICE_VENDOR, valueSize, value, NULL );
			printf( " %d.%d Vendor: %s\n", j + 1, 0, value );
			free( value );

			// print hardware device version
			clGetDeviceInfo( devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize );
			value = (char*) malloc( valueSize );
			clGetDeviceInfo( devices[j], CL_DEVICE_VERSION, valueSize, value, NULL );
			printf( " %d.%d Hardware version: %s\n", j + 1, 1, value );
			free( value );

			// print software driver version
			clGetDeviceInfo( devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize );
			value = (char*) malloc( valueSize );
			clGetDeviceInfo( devices[j], CL_DRIVER_VERSION, valueSize, value, NULL );
			printf( " %d.%d Software version: %s\n", j + 1, 2, value );
			free( value );

			// print c version supported by compiler for device
			clGetDeviceInfo( devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize );
			value = (char*) malloc( valueSize );
			clGetDeviceInfo( devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL );
			printf( " %d.%d OpenCL C version: %s\n", j + 1, 3, value );
			free( value );

			// print parallel compute units
			clGetDeviceInfo( devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
							 sizeof(maxComputeUnits), &maxComputeUnits, NULL );
			printf( " %d.%d Parallel compute units: %d\n", j + 1, 4, maxComputeUnits );


		}
		free( devices );
	}
	free( platforms );
}

char* ReadKernelFromFile( char *filename, int *outLen ) {
	FILE *file;
	fopen_s( &file, filename, "rb" );
	
	// get source length
	fseek( file, 0, SEEK_END );
	int len = ftell( file );
	rewind( file );
	if( outLen != nullptr )
		*outLen = len;

	// read kernel the source
	char *source = (char*) malloc( len + 1 );
	fread_s( source, len, sizeof(char), len, file );
	source[len] = '\0';
	fclose( file );

	return source;
}

char* GetPlatformName(cl_platform_id *platform_id) {
	size_t buff_len;
	clGetPlatformInfo( *platform_id, CL_PLATFORM_NAME, 0, NULL, &buff_len );
	char *buff = (char *) malloc( sizeof(char) *(buff_len + 1) ); buff[buff_len] = '\0';
	clGetPlatformInfo( *platform_id, CL_PLATFORM_NAME, buff_len, buff, NULL );
	return buff;
}

char* GetDeviceName( cl_device_id *device_id ) {
	size_t buff_len;
	clGetDeviceInfo( *device_id, CL_DEVICE_NAME, 0, NULL, &buff_len );
	char *buff = (char *) malloc( sizeof(char) *(buff_len + 1) );  buff[buff_len] = '\0';
	clGetDeviceInfo( *device_id, CL_DEVICE_NAME, buff_len, buff, NULL );
	return buff;
}

void PrintBuildLog(cl_program *program, cl_device_id *device_id) {
	size_t build_log_len;
	char *log;
	clGetProgramBuildInfo( *program, *device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len );
	log = (char *) malloc( sizeof(char) *(build_log_len + 1) );
	clGetProgramBuildInfo( *program, *device_id, CL_PROGRAM_BUILD_LOG, build_log_len, log, NULL );
	printf( "Build log: \n%s\n", log );
	free( log );
}