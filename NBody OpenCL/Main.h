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
	float eps; 				/* konstanta glajenja */
	float dt; 			    /* casovna konstanta */
	int seed;				/* seed za random */

	cl_device_type deviceType;
	size_t local_item_size;
	enum CoordinatesDistributionFunction randFunc;
} info_t;

enum CoordinatesDistributionFunction {
	SPHERE_2_POLES = 0,
	SPHERE = 1
};

// cpu.cpp
void cpu( info_t *info );
void cpuOpt( info_t *info );

// gpu.cpp
void gpu( info_t *info );
void gpuVec( info_t *info );
void gpuVecLocal( info_t *info );

// Helper.cpp
float checkResults( float *X, float *Y, float *Z, int len );
float checkResultsFloat4( float *coord, int len );
void generateCoordinates( float *X, float *Y, float *Z, float *M, info_t *info );
void generateCoordinatesFloat4( float *coord, info_t *info );
void generateCoordinatesSphereFloat4( float *coord, info_t *info );
inline float rand_0_1() {	return rand( ) / (float) RAND_MAX;	}

// mpi.cpp
void mpi( info_t *info );
void mpiOpenCL( info_t *info );