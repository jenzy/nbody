#ifndef _CPU_H
#define _CPU_H

#include <CL\cl.h>

#define M_PI 3.1415926
//#define SWAP(a,b) do {double *temp=a; a=b; b=temp;} while(0)
#define SWAP(a,b) do {float *temp=a; a=b; b=temp;} while(0)

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

// gpu.cpp
void gpu( info_t *info );
void gpuSyncInKernelTest( info_t *info );
void gpuVec( info_t *info );
void gpuVecLocal( info_t *info );

// Helper.cpp
double checkResults( double *X, double *Y, double *Z, int len );
float checkResults( float *X, float *Y, float *Z, int len );
float checkResultsFloat4( float *coord, int len );
void generateCoordinates( double *X, double *Y, double *Z, info_t *info );
void generateCoordinates( float *X, float *Y, float *Z, info_t *info );
void generateCoordinatesFloat4( float *coord, info_t *info );

// HelperOpenCL.cpp
void clinfo( );
char* ReadKernelFromFile( char *filename, int *outLen );
char* GetPlatformName( cl_platform_id *platform_id );
char* GetDeviceName( cl_device_id *device_id );
void PrintBuildLog( cl_program *program, cl_device_id *device_id );

#endif