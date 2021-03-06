#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <GL/freeglut.h>

#include "Main.h"
#include "GL.h"

int width = 1422;
int height = 800;
bool fullscreen = false;

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
    cout << "  -full \t Start in fullscreen" << endl;

    cout << endl;
    cout << "Controls: " << endl;
    cout << "  Arrow keys \t - Rotate camera left/right/up/down" << endl;
    cout << "  +/- \t - Adjust the distance from camera to center" << endl;
    cout << "  Space \t - Pause" << endl;
    cout << "  F \t - Toggle fullscreen" << endl;
    cout << "  Esc \t - Quit" << endl;
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
    info.generateFunc = generateCoordinatesFloat4;

    // Parse Arguments
    for( int i = 1; i < argc; i++ ) {
        if( argv[i][0] == '-' ) {
            if( !strcmp( argv[i], "-help" ) || !strcmp( argv[i], "--help" ) )
                printHelp();
            else if( !strcmp( argv[i], "-n" )  )		info.n				 = atoi( argv[++i] );
            else if( !strcmp( argv[i], "-l" )  )		info.local_item_size = atoi( argv[++i] );
            else if( !strcmp( argv[i], "-w" )  )		width				 = atoi( argv[++i] );
            else if( !strcmp( argv[i], "-h" )  )		height				 = atoi( argv[++i] );
            else if( !strcmp( argv[i], "-t" )  )		info.dt				 = (float) atof( argv[++i] );
            else if( !strcmp( argv[i], "-r" ) )			info.sphereRadius = (float) atof( argv[++i] );
            else if( !strcmp( argv[i], "-r" ) )			fullscreen = true;
            else if( !strcmp( argv[i], "-rnd" )  ) {
                switch( atoi( argv[++i] ) ) {
                    case 0:		info.generateFunc = generateCoordinatesSphereFloat4;	break;
                    case 1:		info.generateFunc = generateCoordinatesFloat4;			break;
                    default:	break;
                }
            } else { printHelp(); exit( 0 ); }
        } else { printHelp(); exit( 0 ); }
    }
    
    glutInit( &argc, argv );
    GL gpu = GL( width, height, &info, fullscreen );
    gpu.Start();

}

/* inicializacija zacetnih polozajev in hitrosti */
/* telesa so na plascu krogle z radijem sphereRadius */
/* hitrost telesa lezi v ravnini, ki je pravokotna na radij */
void generateCoordinatesFloat4( float *coord, info_t *info ) {
    srand( info->seed );
    for( int i = 0; i < info->n; i++ ) {
        const double fiy = 2 * M_PI * rand() / (float)RAND_MAX;
        const double fiz = 2 * M_PI * rand() / (float)RAND_MAX;

        const int index = i * 4;
        coord[index] = (float) (cos( fiz ) * cos( fiy ) * info->sphereRadius);
        coord[index + 1] = (float) (-sin( fiz ) * cos( fiy ) * info->sphereRadius);
        coord[index + 2] = (float) (-sin( fiy ) * info->sphereRadius);
        coord[index + 3] = info->mass;
    }
}
void generateCoordinatesSphereFloat4( float *coord, info_t *info ) {
    srand( info->seed );

    const float r = (float) info->sphereRadius;
    const float r2 = 2 * r;
    const float pi = (float) M_PI;
    const float pi2 = 2 * pi;

    for( int i = 0; i < info->n; i++ ) {
        const float z = r2 * rand_0_1() - r;
        const float phi = pi2 * rand_0_1();
        const float theta = asin(z / r);
        const float rcostheta = r * cos(theta);
        const float x = rcostheta * cos(phi);
        const float y = rcostheta * sin(phi);

        const int index = i * 4;
        coord[index] = x;
        coord[index + 1] = y;
        coord[index + 2] = z;
        coord[index + 3] = info->mass;
    }
}