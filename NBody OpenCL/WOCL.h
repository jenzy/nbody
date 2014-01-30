#pragma once

#include <CL/cl.hpp>
#include <string>

class WOCL {
public:
	WOCL( cl_device_type device_type );
	~WOCL();

private:
	cl_platform_id m_platform_id;
	cl_device_id m_device_id;

	// for checking opencl errors
	cl_int ret = 0;
	bool EXIT_ON_ERROR = true;
	std::string m_id;

public:
	static std::string CLErrorName( cl_int err );
	static std::string GetPlatformName( cl_platform_id *platformId );
	bool CheckForError( cl_int err, std::string name );

	void SetExitOnError( bool exit );
};

