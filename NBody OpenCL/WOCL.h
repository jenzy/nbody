#pragma once

#include <CL/cl.h>

class WOCL {
public:
	WOCL();
	~WOCL();
private:
	cl_context context;
};

