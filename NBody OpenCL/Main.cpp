#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <CL/cl.h>
#include <mpi.h>

#include "Main.h"

int main( int argc, char **argv ) {
	info_t info;
	info.n = 1000; 				
	info.steps = 100;	
	info.sphereRadius = 10; 
	info.kappa = 1; 			
	info.mass = 1; 			
	info.eps = 0.0001f; 		
	info.dt = 0.01f; 	//0.001		
	info.seed = 42;
	info.deviceType = CL_DEVICE_TYPE_GPU;
	info.local_item_size = 64;

	bool doMPI = true;
	bool doCPU = true;
	bool doGPU = false;

	for( int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' ) {
			if( strcmp(argv[i], "-n") == 0 )
				info.n = atoi( argv[++i] );
			else if( strcmp( argv[i], "-s" ) == 0 )
				info.steps = atoi( argv[++i] );
			else if( strcmp( argv[i], "-cpu" ) == 0 )
				doCPU = true;
			else if( strcmp( argv[i], "-nocpu" ) == 0 )
				doCPU = false;
			else if( strcmp( argv[i], "-gpu" ) == 0 )
				doGPU = true;
			else if( strcmp( argv[i], "-nogpu" ) == 0 )
				doGPU = false;
			else if( strcmp( argv[i], "-mpi" ) == 0 )
				doMPI = true;
			else if( strcmp( argv[i], "-nompi" ) == 0 )
				doMPI = false;
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

	if( rank == 0 ) {
		if( doCPU ) {
			cpu( &info );
		}
		if( doGPU ) {
			//gpu( &info );
			//gpuVec( &info );
			gpuVecLocal( &info );
		}
		printf( "\n===========================================================================\n" );
	}


	MPI_Finalize();
}