#pragma once

#include <CL/cl.hpp>
#include <string>
#include <vector>
#include <iostream>

class WOCL {
public:
	WOCL( cl_device_type device_type );
	~WOCL();

private:
	cl_platform_id m_platformId  = nullptr;
	cl_device_id m_deviceId		 = nullptr;
	cl_context m_context		 = nullptr;
	cl_command_queue m_queue	 = nullptr;
	cl_program m_program		 = nullptr;
	cl_kernel m_kernel			 = nullptr;

	std::vector<std::pair<cl_mem, size_t>> m_devBuffers;

	size_t m_localItemSize = 0;
	size_t m_globalItemSize = 0;

	// for checking opencl errors
	cl_int ret = 0;
	bool EXIT_ON_ERROR = true;
	std::string m_id;

public:
	static inline size_t CalculateNumOfGroups( size_t localSize, size_t n ) {
		return ((n - 1) / localSize + 1);
	}
	static inline std::string CLErrorName( cl_int err ) {
		switch( err ) {
			case 0: return "CL_SUCCESS";
			case -1: return "CL_DEVICE_NOT_FOUND";
			case -2: return "CL_DEVICE_NOT_AVAILABLE";
			case -3: return "CL_COMPILER_NOT_AVAILABLE";
			case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
			case -5: return "CL_OUT_OF_RESOURCES";
			case -6: return "CL_OUT_OF_HOST_MEMORY";
			case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
			case -8: return "CL_MEM_COPY_OVERLAP";
			case -9: return "CL_IMAGE_FORMAT_MISMATCH";
			case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
			case -11: return "CL_BUILD_PROGRAM_FAILURE";
			case -12: return "CL_MAP_FAILURE";

			case -30: return "CL_INVALID_VALUE";
			case -31: return "CL_INVALID_DEVICE_TYPE";
			case -32: return "CL_INVALID_PLATFORM";
			case -33: return "CL_INVALID_DEVICE";
			case -34: return "CL_INVALID_CONTEXT";
			case -35: return "CL_INVALID_QUEUE_PROPERTIES";
			case -36: return "CL_INVALID_COMMAND_QUEUE";
			case -37: return "CL_INVALID_HOST_PTR";
			case -38: return "CL_INVALID_MEM_OBJECT";
			case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
			case -40: return "CL_INVALID_IMAGE_SIZE";
			case -41: return "CL_INVALID_SAMPLER";
			case -42: return "CL_INVALID_BINARY";
			case -43: return "CL_INVALID_BUILD_OPTIONS";
			case -44: return "CL_INVALID_PROGRAM";
			case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
			case -46: return "CL_INVALID_KERNEL_NAME";
			case -47: return "CL_INVALID_KERNEL_DEFINITION";
			case -48: return "CL_INVALID_KERNEL";
			case -49: return "CL_INVALID_ARG_INDEX";
			case -50: return "CL_INVALID_ARG_VALUE";
			case -51: return "CL_INVALID_ARG_SIZE";
			case -52: return "CL_INVALID_KERNEL_ARGS";
			case -53: return "CL_INVALID_WORK_DIMENSION";
			case -54: return "CL_INVALID_WORK_GROUP_SIZE";
			case -55: return "CL_INVALID_WORK_ITEM_SIZE";
			case -56: return "CL_INVALID_GLOBAL_OFFSET";
			case -57: return "CL_INVALID_EVENT_WAIT_LIST";
			case -58: return "CL_INVALID_EVENT";
			case -59: return "CL_INVALID_OPERATION";
			case -60: return "CL_INVALID_GL_OBJECT";
			case -61: return "CL_INVALID_BUFFER_SIZE";
			case -62: return "CL_INVALID_MIP_LEVEL";
			case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
			default: return "Unknown OpenCL error";
		}
	}
	static std::string GetPlatformName( cl_platform_id *platformId );
	static std::string GetDeviceName( cl_device_id *deviceId );
	static std::string GetBuildLog( cl_program *program, cl_device_id *device_id );
	static char* ReadWholeFile( char *filename, int *outLen );

	bool CheckForError( cl_int err, std::string name );
	void PrintError( std::string error );

	void SetWorkSize( size_t numItemsPerGroup, size_t numGroups, size_t numItemsGlobal );
	void CreateAndBuildKernel( char *filename, char *functionName );

	cl_mem CreateBuffer( size_t size, cl_mem_flags flags, void * hostBuffer );
	void CopyDeviceToHost( cl_mem *device, void *host, size_t size );
	void ExecuteKernel();

	template<typename T> void SetKernelArgument( int idx, T *parameter ) {
		ret = clSetKernelArg( m_kernel, idx, sizeof(T), (void *) parameter );
		CheckForError( ret, "clSetKernelArg" );
	}
};

