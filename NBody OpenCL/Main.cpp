#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mpi.h>
#include <GL/freeglut.h>

#include "Main.h"
#include "GL.h"

void printHelp() {
	using namespace std;
	cout << "Arguments: " << endl;
	cout << "  -n <int> \t Set number of bodies" << endl;
	cout << "  -s <int> \t Set number of steps" << endl;
	cout << "  -t <float> \t Set dt" << endl;
	cout << "  -l <int> \t Set local work size (GPU)" << endl;
	cout << "  -rnd <int> \t Random function for generating coordinates (used only if GL)" << endl;
	cout << "  \t\t   Possible values: 0: SPHERE, 1: SPHERE_2_POLES" << endl;
	cout << "  -cpu and -nocpu \t Run (or don't) the simulation on CPU" << endl;
	cout << "  -cpuopt, -nocpuopt \t Run (or don't) the simulation on CPU - the optimised version" << endl;
	cout << "  -gpu1 and -nogpu1 \t Run (or don't) the simulation on GPU - the basic version" << endl;
	cout << "  -gpu2 and -nogpu2 \t Run (or don't) the simulation on GPU - the vectorized version" << endl;
	cout << "  -gpu3 and -nogpu3 \t Run (or don't) the simulation on GPU - the vectorized version with the use of local memory" << endl;
	cout << "  -mpi and -nompi \t Run (or don't) the simulation with MPI - you really should use mpiexec..." << endl;
	cout << "  -combo and -nocombo \t Run (or don't) the simulation with MPI and OpenCL - you really should use mpiexec..." << endl;
	cout << "  -GL and -noGL \t Display (or don't) the simulation with OpenGL and OpenCL - so pretty" << endl;
}

int main( int argc, char **argv ) {
	info_t info;
	info.n = 10240; 				
	info.steps = 1000;	
	info.sphereRadius = 10; //10
	info.kappa = 1; 			
	info.mass = 1; 			
	info.eps = 0.0001f; 		
	info.dt = 0.001f; 	//0.001		
	info.seed = 42;
	info.deviceType = CL_DEVICE_TYPE_GPU;
	info.local_item_size = 256;
	info.randFunc = SPHERE;

	bool doMPI = false;
	bool doCPU = false;
	bool doCPUOpt = false;
	bool doGPU1 = false;
	bool doGPU2 = false;
	bool doGPU3 = false;
	bool doCombo = false;
	bool doGL = true;

#pragma region Parse Arguments
	for( int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' ) {
			if( strcmp( argv[i], "-n" ) == 0 )
				info.n = atoi( argv[++i] );
			else if( strcmp( argv[i], "-l" ) == 0 )
				info.local_item_size = atoi( argv[++i] );
			else if( strcmp( argv[i], "-t" ) == 0 )
				info.dt = atof( argv[++i] );
			else if( strcmp( argv[i], "-s" ) == 0 )
				info.steps = atoi( argv[++i] );
			else if( strcmp( argv[i], "-rnd" ) == 0 ) {
				switch( atoi( argv[++i] ) ) {
					case 0:
						info.randFunc = SPHERE;
						break;
					case 1:
						info.randFunc = SPHERE_2_POLES;
						break;
					default:
						break;
				}
			}
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
		} else {
			printHelp();
			exit( 0 );
		}
	}

	if( !doCPU && !doCPUOpt && !doGPU1 && !doGPU2 && !doGPU3 && !doMPI && !doCombo && !doGL ) {
		printHelp( );
		exit( 0 );
	}

#pragma endregion

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
			GL gpu = GL( 800, 800, &info );
			gpu.Play();
		}

		std::cout << std::endl << "===========================================================================" << std:: endl;
	}

	MPI_Finalize();
}