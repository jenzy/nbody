#pragma once
#include <CL\cl.h>

#define M_PI 3.1415926
#define SWAP_DOUBLE(a,b) do {double *temp=a; a=b; b=temp;} while(0)
#define SWAP(a,b) do {float *temp=a; a=b; b=temp;} while(0)
#define SWAP_MEM(a,b) do {cl_mem temp=a; a=b; b=temp;} while(0)

typedef struct _info {
	int n; 			        /* stevilo teles */
	int steps; 			    /* stevilo korakov */
	double sphereRadius; 	/* nastavitve zacetne konfiguracije */
	float kappa; 			/* gravitacijska konstanta */
	float mass; 			/* masa teles */
	float eps; 		    /* konstanta glajenja */
	float dt; 			    /* casovna konstanta */
	int seed;				/* seed za random */

	cl_device_type deviceType;
	size_t local_item_size;
} info_t;

// cpu.cpp
void cpu( info_t *info );
void cpuOpt( info_t *info );

// gpu.cpp
void gpu( info_t *info );
void gpuVec( info_t *info );
void gpuVecLocal( info_t *info );
void gpuOpenGL( info_t *info );

// Helper.cpp
float checkResults( float *X, float *Y, float *Z, int len );
float checkResultsFloat4( float *coord, int len );
void generateCoordinates( float *X, float *Y, float *Z, float *M, info_t *info );
void generateCoordinatesFloat4( float *coord, info_t *info );

// HelperOpenCL.cpp
void clinfo( );
char* ReadKernelFromFile( char *filename, int *outLen );
char* GetPlatformName( cl_platform_id *platform_id );
char* GetDeviceName( cl_device_id *device_id );
void PrintBuildLog( cl_program *program, cl_device_id *device_id );
void PrintDeviceInfo( cl_platform_id *platform_id, cl_device_id *device_id );
void BuildKernel( cl_program *program, cl_context *context, cl_device_id *device, char *filename );

// mpi.cpp
void mpi( info_t *info );
void mpiOpenCL( info_t *info );