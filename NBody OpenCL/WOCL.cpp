#include <CL/cl.hpp>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include "WOCL.h"


WOCL::WOCL( cl_device_type deviceType ) {
	this->m_id = "WOCL";
	
	ret = clGetPlatformIDs( 1, &m_platformId, NULL );									CheckForError( ret, "clGetPlatformIDs" );
	ret = clGetDeviceIDs( m_platformId, deviceType, 1, &m_deviceId, NULL );				CheckForError( ret, "clGetDeviceIDs" );
	
	std::cout << "Naprava: " << GetDeviceName(&m_deviceId) << " (" << GetPlatformName( &m_platformId ) << ")" << std::endl;

	m_context = clCreateContext( NULL, 1, &m_deviceId, NULL, NULL, &ret );				CheckForError( ret, "clCreateContext" );
	m_queue = clCreateCommandQueue( m_context, m_deviceId, 0, &ret );					CheckForError( ret, "clCreateCommandQueue" );
}
WOCL::~WOCL() {
	ret = clFlush( m_queue );
	ret |= clFinish( m_queue );

	for( int i = 0; i < m_devBuffers.size(); i++ ) {
		ret |= clReleaseMemObject( m_devBuffers[i].first );
	}

	ret |= clReleaseKernel( m_kernel );
	ret |= clReleaseProgram( m_program );
	ret |= clReleaseCommandQueue( m_queue );
	ret |= clReleaseContext( m_context );
	CheckForError( ret, "Cleanup" );
}

bool WOCL::CheckForError( cl_int err, std::string name ) {
	if( err == CL_SUCCESS ) return false;
	else {
		std::cout << "ERROR: " << CLErrorName( err ) << "    " << name << "@" << this->m_id << std::endl;
		if(EXIT_ON_ERROR) exit( err );
		return true;
	}
}
void WOCL::PrintError( std::string error ) {
	std::cout << "ERROR: " << error << std::endl;
	if( EXIT_ON_ERROR ) exit( -1 );
}

std::string WOCL::GetPlatformName( cl_platform_id *platformId ) {
	size_t buff_len;
	clGetPlatformInfo( *platformId, CL_PLATFORM_NAME, 0, NULL, &buff_len );
	char *buff = (char *) malloc( sizeof(char) *(buff_len + 1) ); buff[buff_len] = '\0';
	clGetPlatformInfo( *platformId, CL_PLATFORM_NAME, buff_len, buff, NULL );

	std::string name( buff );
	free( buff );
	return name;
}
std::string WOCL::GetDeviceName( cl_device_id *deviceId ) {
	size_t buff_len;
	clGetDeviceInfo( *deviceId, CL_DEVICE_NAME, 0, NULL, &buff_len );
	char *buff = (char *) malloc( sizeof(char) *(buff_len + 1) );  buff[buff_len] = '\0';
	clGetDeviceInfo( *deviceId, CL_DEVICE_NAME, buff_len, buff, NULL );
	
	std::string name( buff );
	free( buff );
	return name;
}
std::string WOCL::GetBuildLog( cl_program *program, cl_device_id *device_id ) {
	size_t build_log_len;
	char *log;
	clGetProgramBuildInfo( *program, *device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len );
	log = (char *) malloc( sizeof(char) *(build_log_len + 1) );
	clGetProgramBuildInfo( *program, *device_id, CL_PROGRAM_BUILD_LOG, build_log_len, log, NULL ); log[build_log_len] = '\0';
	std::string tmp( log );
	free( log );
	return tmp;
}
char* WOCL::ReadWholeFile( char *filename, int *outLen ) {
	FILE *file;
	fopen_s( &file, filename, "rb" );
	if( file == NULL ) {
		std::cout << "ERROR while opening file" << filename << std::endl;
		exit( -1 );
	}

	// get length
	fseek( file, 0, SEEK_END );
	int len = ftell( file );
	rewind( file );
	if( outLen != nullptr )
		*outLen = len;

	// read file
	char *source = (char*) malloc( len + 1 );
	fread_s( source, len, sizeof(char), len, file );
	source[len] = '\0';
	fclose( file );

	return source;
}

void WOCL::SetWorkSize( size_t numItemsPerGroup, size_t numGroups, size_t numItemsGlobal ) {
	if( numItemsPerGroup == 0 )		PrintError( "Invalid parameters to SetWorkSize, numItemsPerGroup can't be 0." );
	m_localItemSize = numItemsPerGroup;

	if( numGroups != 0 ) {
		m_globalItemSize = m_localItemSize * numGroups;
	} else {
		if( numItemsGlobal == 0 )
			PrintError( "Invalid parameters to SetWorkSize, numGroups and numItemsGlobal can't both be 0." );
		m_globalItemSize = numItemsGlobal;
	}

	printf( "Delitev dela: local: %d | num_groups: %d | global: %d\n",
			m_localItemSize, m_globalItemSize / m_localItemSize, m_globalItemSize );
}
void WOCL::CreateAndBuildKernel( char *filename, char *functionName ) {
	char *source_str = ReadWholeFile( filename, NULL );
	m_program = clCreateProgramWithSource( m_context, 1, (const char **) &source_str, NULL, &ret );			CheckForError( ret, "clCreateProgramWithSource" );
	
	ret = clBuildProgram( m_program, 1, &m_deviceId, NULL, NULL, NULL );
	if( ret != CL_SUCCESS ) {
		std::cout << "Build log: " << std::endl << WOCL::GetBuildLog( &m_program, &m_deviceId ) << std::endl;
		CheckForError( ret, "clBuildProgram" );
	}
	free( source_str );

	m_kernel = clCreateKernel( m_program, functionName, &ret );												CheckForError( ret, "clCreateKernel" );
}
cl_mem WOCL::CreateBuffer( size_t size, cl_mem_flags flags, void *hostBuffer ) {
	cl_mem buff = clCreateBuffer( m_context, flags, size, hostBuffer, &ret );		CheckForError( ret, "clcreateBuffer" );
	m_devBuffers.push_back( std::make_pair( buff, size ) );
	return buff;
}
void WOCL::CopyDeviceToHost( cl_mem *device, void *host, size_t size ) {
	ret = clEnqueueReadBuffer( m_queue, *device, CL_TRUE, 0, size, host, 0, NULL, NULL );
	CheckForError( ret, "clEnqueueReadBuffer" );
}
void WOCL::SetAndAllocKernelArgument( int idx, size_t size ) {
	ret = clSetKernelArg( m_kernel, idx, size, NULL);
	CheckForError( ret, "clSetKernelArg (alloc)" );
}
void WOCL::ExecuteKernel() {
	ret = clEnqueueNDRangeKernel( m_queue, m_kernel, 1, NULL, &m_globalItemSize, &m_localItemSize, 0, NULL, NULL );
	CheckForError( ret, "clEnqueueNDRangeKernel" );
}
