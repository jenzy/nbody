#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <CL/cl.h>
#include <mpi.h>
#include <GL/freeglut.h>

#include "Main.h"

int main( int argc, char **argv ) {
	info_t info;
	info.n = 1000; 				
	info.steps = 1000;	
	info.sphereRadius = 10; //10
	info.kappa = 1; 			
	info.mass = 1; 			
	info.eps = 0.0001f; 		
	info.dt = 0.01f; 	//0.001		
	info.seed = 42;
	info.deviceType = CL_DEVICE_TYPE_GPU;
	info.local_item_size = 256;

	bool doMPI = false;
	bool doCPU = false;
	bool doCPUOpt = false;
	bool doGPU1 = true;
	bool doGPU2 = true;
	bool doGPU3 = false;
	bool doCombo = false;
	bool doGL = false;

	for( int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' ) {
			if( strcmp( argv[i], "-n" ) == 0 )
				info.n = atoi( argv[++i] );
			if( strcmp( argv[i], "-l" ) == 0 )
				info.local_item_size = atoi( argv[++i] );
			else if( strcmp( argv[i], "-s" ) == 0 )
				info.steps = atoi( argv[++i] );
			else if( strcmp( argv[i], "-cpu" ) == 0 )
				doCPU = true;
			else if( strcmp( argv[i], "-nocpu" ) == 0 )
				doCPU = false;
			else if( strcmp( argv[i], "-cpuopt" ) == 0 )
				doCPUOpt = true;
			else if( strcmp( argv[i], "-nocpuopt" ) == 0 )
				doCPUOpt = false;
			else if( strcmp( argv[i], "-gpu1" ) == 0 )
				doGPU1 = true;
			else if( strcmp( argv[i], "-nogpu1" ) == 0 )
				doGPU1 = false;
			else if( strcmp( argv[i], "-gpu2" ) == 0 )
				doGPU2 = true;
			else if( strcmp( argv[i], "-nogpu2" ) == 0 )
				doGPU2 = false;
			else if( strcmp( argv[i], "-gpu3" ) == 0 )
				doGPU3 = true;
			else if( strcmp( argv[i], "-nogpu3" ) == 0 )
				doGPU3 = false;
			else if( strcmp( argv[i], "-mpi" ) == 0 )
				doMPI = true;
			else if( strcmp( argv[i], "-nompi" ) == 0 )
				doMPI = false;
			else if( strcmp( argv[i], "-combo" ) == 0 )
				doCombo = true;
			else if( strcmp( argv[i], "-nocombo" ) == 0 )
				doCombo = false;
			else if( strcmp( argv[i], "-GL" ) == 0 )
				doGL = true;
			else if( strcmp( argv[i], "-noGL" ) == 0 )
				doGL = false;
		}
	}

	int rank;
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	if( doMPI ) {
		mpi( &info );
		MPI_Barrier( MPI_COMM_WORLD );
		fflush( stdout );
	}

	if( doCombo ) {
		mpiOpenCL( &info );
		MPI_Barrier( MPI_COMM_WORLD );
		fflush( stdout );
	}

	if( rank == 0 ) {
		if( doCPU )  cpu( &info );
		if( doCPUOpt) cpuOpt( &info );
		if( doGPU1 ) gpu( &info );
		if( doGPU2 ) gpuVec( &info );
		if( doGPU3 ) gpuVecLocal( &info ); 
		if( doGL ) {
			glutInit( &argc, argv );
			gpuOpenGL( &info );
		}

		printf( "\n===========================================================================\n" );
	}

	MPI_Finalize();
}