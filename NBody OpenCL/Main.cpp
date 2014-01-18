#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <CL/cl.h>
#include <mpi.h>

#include "Main.h"

int main( int argc, char **argv ) {
	info_t info;
	info.n = 1000; 				
	info.steps = 100; 		//10	
	info.sphereRadius = 10; 
	info.kappa = 1; 			
	info.mass = 1; 			
	info.eps = 0.0001; 		
	info.dt = 0.01; 	//0.001		
	info.seed = 42;

	info.deviceType = CL_DEVICE_TYPE_GPU;
	info.local_item_size = 64;

	int rank;
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	mpi( &info );

	MPI_Barrier( MPI_COMM_WORLD );
	fflush( stdout );

	if( rank == 0 ) {
		cpu( &info );
		//gpu( &info );
		//gpuVec( &info );
		gpuVecLocal( &info );
	}


	MPI_Finalize();
}