#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <GL/freeglut.h>

#include "Main.h"
#include "GL.h"

int width = 1422;
int height = 800;

void printHelp() {
	using namespace std;
	cout << "Arguments: " << endl;
	cout << "  -n <int> \t Set number of bodies" << endl;
	cout << "  -t <float> \t Set dt" << endl;
	cout << "  -l <int> \t Set local work size (GPU)" << endl;
	cout << "  -w <int> \t Set width of the window (in px)" << endl;
	cout << "  -h <int> \t Set height of the window (in px)" << endl;
	cout << "  -r <float> \t Set the radius of the starting sphere" << endl;
	cout << "  -rnd <int> \t Random function for generating coordinates (used only if GL)" << endl;
	cout << "  \t\t   Possible values: 0: SPHERE, 1: SPHERE_2_POLES" << endl;
}

int main( int argc, char **argv ) {
	info_t info;
	info.n = 10240;
	info.sphereRadius = 17.f;
	info.kappa = 1;
	info.mass = 1;
	info.eps = 0.0001f;
	info.dt = 0.001f;	
	info.seed = 42;
	info.deviceType = CL_DEVICE_TYPE_GPU;
	info.local_item_size = 256;
	info.generateFunc = generateCoordinatesSphereFloat4;

#pragma region Parse Arguments
	for( int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' ) {
			if( strcmp( argv[i], "-n" ) == 0 )			info.n				 = atoi( argv[++i] );
			else if( strcmp( argv[i], "-l" ) == 0 )		info.local_item_size = atoi( argv[++i] );
			else if( strcmp( argv[i], "-w" ) == 0 )		width				 = atoi( argv[++i] );
			else if( strcmp( argv[i], "-h" ) == 0 )		height				 = atoi( argv[++i] );
			else if( strcmp( argv[i], "-t" ) == 0 )		info.dt				 = (float) atof( argv[++i] );
			else if( strcmp( argv[i], "-r" ) == 0 )		info.sphereRadius	 = (float) atof( argv[++i] );
			else if( strcmp( argv[i], "-rnd" ) == 0 ) {
				switch( atoi( argv[++i] ) ) {
					case 0:
						info.generateFunc = generateCoordinatesSphereFloat4;
						break;
					case 1:
						info.generateFunc = generateCoordinatesFloat4;
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
	GL gpu = GL( width, height, &info );
	gpu.Start();

}