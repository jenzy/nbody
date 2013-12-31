#ifndef _CPU_H
#define _CPU_H

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
} info_t;

// cpu.cpp
void cpuOriginal( info_t *info );

// gpu.cpp
void gpu( void );

// Helper.cpp
double checkResults( double *X, double *Y, double *Z, int len );
void generateCoordinates( double *X, double *Y, double *Z, info_t *info );

// HelperOpenCL.cpp
void clinfo( );

#endif