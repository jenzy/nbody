#ifndef _CPU_H
#define _CPU_H

#include <CL\cl.h>

#define M_PI 3.1415926
#define SWAP(a,b) do {double *temp=a; a=b; b=temp;} while(0)

typedef struct _info {
	int n; 			        /* stevilo teles */
	int steps; 			    /* stevilo korakov */
	double sphereRadius; 	/* nastavitve zacetne konfiguracije */
	double kappa; 			/* gravitacijska konstanta */
	double mass; 			/* masa teles */
	double eps; 		    /* konstanta glajenja */
	double dt; 			    /* casovna konstanta */
	int seed;				/* seed za random */

	cl_device_type deviceType;
} info_t;

// cpu.cpp
void cpuOriginal( info_t *info );

// gpu.cpp
void gpu( info_t *info );

// Helper.cpp
double checkResults( double *X, double *Y, double *Z, int len );
float checkResults( float *X, float *Y, float *Z, int len );
void generateCoordinates( double *X, double *Y, double *Z, info_t *info );
void generateCoordinates( float *X, float *Y, float *Z, info_t *info );

// HelperOpenCL.cpp
void clinfo( );
char* ReadKernelFromFile( char *filename, int *outLen );
char* GetPlatformName( cl_platform_id *platform_id );
char* GetDeviceName( cl_device_id *device_id );
void PrintBuildLog( cl_program *program, cl_device_id *device_id );

#endif