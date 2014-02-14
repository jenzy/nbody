#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <GL/freeglut.h>

#include "Main.h"
#include "GL.h"

void printHelp() {
	using namespace std;
	cout << "Arguments: " << endl;
	cout << "  -n <int> \t Set number of bodies" << endl;
	cout << "  -t <float> \t Set dt" << endl;
	cout << "  -l <int> \t Set local work size (GPU)" << endl;
	cout << "  -rnd <int> \t Random function for generating coordinates (used only if GL)" << endl;
	cout << "  \t\t   Possible values: 0: SPHERE, 1: SPHERE_2_POLES" << endl;
}

int main( int argc, char **argv ) {
	info_t info;
	info.n = 10000;
	info.sphereRadius = 10; //10
	info.kappa = 1;
	info.mass = 1;
	info.eps = 0.0001f;
	info.dt = 0.001f; 	//0.001		
	info.seed = 42;
	info.deviceType = CL_DEVICE_TYPE_GPU;
	info.local_item_size = 256;
	info.randFunc = SPHERE_2_POLES;

#pragma region Parse Arguments
	for( int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' ) {
			if( strcmp( argv[i], "-n" ) == 0 )
				info.n = atoi( argv[++i] );
			else if( strcmp( argv[i], "-l" ) == 0 )
				info.local_item_size = atoi( argv[++i] );
			else if( strcmp( argv[i], "-t" ) == 0 )
				info.dt = (float) atof( argv[++i] );
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
		} else {
			printHelp();
			exit( 0 );
		}
	}
#pragma endregion
	
	glutInit( &argc, argv );
	GL gpu = GL( 800, 800, &info );
	gpu.Start();

}