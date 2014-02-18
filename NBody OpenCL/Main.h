#pragma once
#include <CL\cl.h>

#define M_PI 3.1415926

struct info_t {
	int n; 			        /* stevilo teles */
	float sphereRadius; 	/* nastavitve zacetne konfiguracije */
	float kappa; 			/* gravitacijska konstanta */
	float mass; 			/* masa teles */
	float eps; 				/* konstanta glajenja */
	float dt; 			    /* casovna konstanta */
	int seed;				/* seed za random */

	cl_device_type deviceType;
	size_t local_item_size;
	void ( *generateFunc )(float*, info_t*);		/* function which generates the initial coordinates of the particles */
};
typedef info_t info_t;

// Helper.cpp
void generateCoordinatesFloat4( float *coord, info_t *info );
void generateCoordinatesSphereFloat4( float *coord, info_t *info );
inline float rand_0_1() {	return rand( ) / (float) RAND_MAX;	}
