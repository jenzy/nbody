#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/cl_gl.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Main.h"
#include "Timer.h"
#include "WOCL.h"

void gpu( info_t *info ) {
	printf( "\n\n== OpenCL ==                   N: %d, Steps: %d\n", info->n, info->steps );

	Timer t;
	WOCL gpu = WOCL( CL_DEVICE_TYPE_GPU );
	gpu.SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, info->n ), 0 );

	// Host alokacija
	float *M = (float *) malloc( sizeof(float) * info->n );
	float *X = (float *) malloc( sizeof(float) * info->n );
	float *Y = (float *) malloc( sizeof(float) * info->n );
	float *Z = (float *) malloc( sizeof(float) * info->n );
	float *V = (float *) calloc( info->n, sizeof(float) );
	generateCoordinates( X, Y, Z, M, info );

	t.Tic();
	gpu.CreateAndBuildKernel( "kernelFloat1.cl", "kernelFloat1" );

	// Device alokacija in kopiranje podatkov
	cl_mem devX = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, X );
	cl_mem devY = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Y );
	cl_mem devZ = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Z );
	cl_mem devM = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, M );
	cl_mem devNewX = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE, NULL );
	cl_mem devNewY = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE, NULL );
	cl_mem devNewZ = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE, NULL );
	cl_mem devVX = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );
	cl_mem devVY = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );
	cl_mem devVZ = gpu.CreateBuffer( info->n*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );

	gpu.SetKernelArgument<cl_mem>( 6, &devVX );
	gpu.SetKernelArgument<cl_mem>( 7, &devVY );
	gpu.SetKernelArgument<cl_mem>( 8, &devVZ );
	gpu.SetKernelArgument<cl_mem>( 9, &devM );
	gpu.SetKernelArgument<cl_int>( 10, &(info->n) );
	gpu.SetKernelArgument<cl_float>( 11, &(info->eps) );
	gpu.SetKernelArgument<cl_float>( 12, &(info->kappa) );
	gpu.SetKernelArgument<cl_float>( 13, &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		gpu.SetKernelArgument<cl_mem>( 0, &devX );
		gpu.SetKernelArgument<cl_mem>( 1, &devY );
		gpu.SetKernelArgument<cl_mem>( 2, &devZ );
		gpu.SetKernelArgument<cl_mem>( 3, &devNewX );
		gpu.SetKernelArgument<cl_mem>( 4, &devNewY );
		gpu.SetKernelArgument<cl_mem>( 5, &devNewZ );

		gpu.ExecuteKernel();

		SWAP_MEM( devNewX, devX );
		SWAP_MEM( devNewY, devY );
		SWAP_MEM( devNewZ, devZ );
	}

	// Prenos rezultatov na gostitelja
	gpu.CopyDeviceToHost( &devX, X, info->n*sizeof(float) );
	gpu.CopyDeviceToHost( &devY, Y, info->n*sizeof(float) );
	gpu.CopyDeviceToHost( &devZ, Z, info->n*sizeof(float) );

	printf( "Cas izvajanja: %lf\n", t.Toc() );
	checkResults( X, Y, Z, info->n );

#pragma region Cleanup
	free( M );
	free( X );
	free( Y );
	free( Z );
#pragma endregion
}

void gpuVec( info_t *info ) {
	printf( "\n\n== OpenCL (float4) ==          N: %d, Steps: %d\n", info->n, info->steps );

	Timer t;
	WOCL gpu = WOCL( CL_DEVICE_TYPE_GPU );
	gpu.SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, info->n ), 0 );

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V = (float *) calloc( info->n, 4 * sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	t.Tic( );
	gpu.CreateAndBuildKernel( "kernelVec.cl", "kernelVec" );

	// Device alokacija in kopiranje podatkov
	cl_mem devCoord		= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Coord );
	cl_mem devCoordNew	= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE,						 NULL );
	cl_mem devV			= gpu.CreateBuffer( info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );

	gpu.SetKernelArgument<cl_mem>( 2, &devV );
	gpu.SetKernelArgument<cl_int>( 3, &(info->n) );
	gpu.SetKernelArgument<cl_float>( 4, &(info->eps) );
	gpu.SetKernelArgument<cl_float>( 5, &(info->kappa) );
	gpu.SetKernelArgument<cl_float>( 6, &(info->dt) );

	// šcepec: zagon
	for( int i = 0; i < info->steps; i++ ) {
		gpu.SetKernelArgument<cl_mem>( 0, &devCoord );
		gpu.SetKernelArgument<cl_mem>( 1, &devCoordNew );

		gpu.ExecuteKernel( );

		SWAP_MEM( devCoord, devCoordNew);
	}

	// Prenos rezultatov na gostitelja
	gpu.CopyDeviceToHost( &devCoord, Coord, info->n*sizeof(cl_float4) );

	printf( "Cas izvajanja: %lf\n", t.Toc() );
	checkResultsFloat4( Coord, info->n );

	free( V );
	free( Coord );
}

void gpuVecLocal( info_t *info ) {
	cl_int	ret;
	Timer time;
	printf( "\n\n== OpenCL (float4 local) ==\n" );

#pragma region OpenCL Inicializacija
	// Platforma in naprava
	cl_platform_id platform_id;
	cl_device_id device_id;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	PrintDeviceInfo( &platform_id, &device_id );

	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

#pragma region Delitev dela
	// Delitev dela
	size_t local_item_size = info->local_item_size;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d, steps: %d)\n", local_item_size, num_groups, global_item_size, info->n, info->steps );
#pragma endregion

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(float) * 4 * info->n );
	float *V =    (float *) calloc( info->n, 4 * sizeof(float) );
	generateCoordinatesFloat4( Coord, info );

	time.Tic();
	// Device alokacija in kopiranje podatkov
	cl_mem devCoord    = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), Coord, &ret );
	cl_mem devCoordNew = clCreateBuffer( context, CL_MEM_READ_WRITE,                        info->n*sizeof(cl_float4), NULL,  &ret );
	cl_mem devV        = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), V,     &ret );

	// Priprava programa
	cl_program program;
	BuildKernel( &program, &context, &device_id, "kernelVecLocal.cl" );
	
	// priprava šcepca 
	cl_kernel krnl = clCreateKernel( program, "kernelVecLocal", &ret );
	ret |= clSetKernelArg( krnl, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( krnl, 3, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( krnl, 4, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( krnl, 5, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( krnl, 6, sizeof(cl_float), (void *) &(info->dt) );
	ret |= clSetKernelArg( krnl, 7, info->local_item_size * sizeof(cl_float4), NULL );

	// zagon šèepca
	for( int i = 0; i < info->steps; i++ ) {
		ret = clSetKernelArg( krnl, 0, sizeof(cl_mem), (void *) &devCoord );
		ret |= clSetKernelArg( krnl, 1, sizeof(cl_mem), (void *) &devCoordNew );
		ret |= clEnqueueNDRangeKernel( command_queue, krnl, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
		SWAP_MEM( devCoord, devCoordNew );
	}

	// Prenos rezultatov na gostitelja
	ret = clEnqueueReadBuffer( command_queue, devCoord, CL_TRUE, 0, info->n*sizeof(cl_float4), Coord, 0, NULL, NULL );

	printf( "Cas izvajanja %lf\n", time.Toc());
	checkResultsFloat4( Coord, info->n );

#pragma region Cleanup
	ret = clFlush( command_queue );
	ret = clFinish( command_queue );
	ret = clReleaseKernel( krnl );
	ret = clReleaseProgram( program );
	ret = clReleaseMemObject( devV );
	ret = clReleaseMemObject( devCoord );
	ret = clReleaseMemObject( devCoordNew );
	ret = clReleaseCommandQueue( command_queue );
	ret = clReleaseContext( context );

	free( V );
	free( Coord );
#pragma endregion
}

void timerCB( int ms ) {
	//this makes sure the appRender function is called every ms miliseconds
	glutTimerFunc( ms, timerCB, ms );
	glutPostRedisplay( );
}

GLuint vboCoord = 0;
int n = 0;
cl_mem devCoordNew;
cl_mem devCoord;
cl_kernel krnl;
cl_command_queue command_queue;
cl_int ret;
size_t global_item_size;
size_t local_item_size;

void render( void ) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glFinish( );
	ret = clEnqueueAcquireGLObjects( command_queue, 1, &devCoord, 0, NULL, NULL );
	ret = clFinish( command_queue );

	//execute the kernel
	ret = clSetKernelArg( krnl, 0, sizeof(cl_mem), (void *) &devCoord );
	ret |= clSetKernelArg( krnl, 1, sizeof(cl_mem), (void *) &devCoordNew );
	ret |= clEnqueueNDRangeKernel( command_queue, krnl, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL );
	//printf( "hi\n" );
	clFinish( command_queue );

	//Release the VBOs so OpenGL can play with them
	clEnqueueReleaseGLObjects( command_queue, 1, &devCoord, 0, NULL, NULL );
	clFinish( command_queue );

	SWAP_MEM( devCoord, devCoordNew );

	//render the particles from VBOs
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_POINT_SMOOTH );
	glPointSize( 2. );
	glColor3f( 1.0f, 1.0f, 1.0f );

	//printf("color buffer\n");
	//glBindBuffer( GL_ARRAY_BUFFER, example->c_vbo );
	//glColorPointer( 4, GL_FLOAT, 0, 0 );

	//printf("vertex buffer\n");
	glBindBuffer( GL_ARRAY_BUFFER, vboCoord );
	glVertexPointer( 3, GL_FLOAT, sizeof(float), 0 );

	//printf("enable client state\n");
	glEnableClientState( GL_VERTEX_ARRAY );
	//glEnableClientState( GL_COLOR_ARRAY );

	//Need to disable these for blender
	glDisableClientState( GL_NORMAL_ARRAY );

	//printf("draw arrays\n");
	glDrawArrays( GL_POINTS, 0, n );

	//printf("disable stuff\n");
	//glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );

	glutSwapBuffers( );
}


void gpuOpenGL( info_t *info ) {

#pragma region Delitev dela
	// Delitev dela
	local_item_size = info->local_item_size;
	size_t num_groups = ((info->n - 1) / local_item_size + 1);
	global_item_size = num_groups*local_item_size;
	printf( "Delitev dela: local: %d | num_groups: %d | global: %d  (N: %d, steps: %d)\n", local_item_size, num_groups, global_item_size, info->n, info->steps );
#pragma endregion

#pragma region GL Init
	int window_width = 800, window_height = 800;
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( window_width, window_height );
	glutInitWindowPosition( glutGet( GLUT_SCREEN_WIDTH ) / 2 - window_width / 2, glutGet( GLUT_SCREEN_HEIGHT ) / 2 - window_height / 2 );
	glutCreateWindow( "N-Body" );

	glutDisplayFunc( render ); //main rendering function
	glutTimerFunc( 30, timerCB, 30 ); //determin a minimum time between frames
	//glutKeyboardFunc( appKeyboard );
	//glutMouseFunc( appMouse );
	//glutMotionFunc( appMotion );

	glewInit( );

	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glDisable( GL_DEPTH_TEST );

	// projection
	glViewport( 0, 0, window_width, window_height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 90.0, (GLfloat) window_width / (GLfloat) window_height, 0.1, 1000.0 );

	// view matrix
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glTranslatef( 0.0, 0.0, -25.0f );
#pragma endregion

#pragma region OpenCL Inicializacija
	cl_platform_id platform_id;
	cl_device_id device_id;
	ret = clGetPlatformIDs( 1, &platform_id, NULL );
	ret = clGetDeviceIDs( platform_id, info->deviceType, 1, &device_id, NULL );
	PrintDeviceInfo( &platform_id, &device_id );

	//cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret );			// Kontekst
	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext( ),
		CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC( ),
		CL_CONTEXT_PLATFORM, (cl_context_properties) (platform_id),
		0
	};
	cl_context context = clCreateContext(props, 1, &device_id, NULL, NULL, &ret);
	command_queue = clCreateCommandQueue( context, device_id, 0, &ret );	// Ukazna vrsta
#pragma endregion

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(cl_float4) * info->n );
	generateCoordinatesFloat4( Coord, info );
	float *V = (float *) calloc( info->n, 4 * sizeof(float) );
	n = info->n;

	glGenBuffers( 1, &vboCoord );
	glBindBuffer( GL_ARRAY_BUFFER, vboCoord );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cl_float4) * info->n, Coord, GL_DYNAMIC_DRAW ); // upload data to video card

	// Device alokacija in kopiranje podatkov
	devCoord = clCreateFromGLBuffer( context, CL_MEM_READ_WRITE, vboCoord, &ret );
	printf( "%d\n", ret );
	devCoordNew = clCreateBuffer( context, CL_MEM_READ_WRITE, info->n*sizeof(cl_float4), NULL, &ret );
	cl_mem devV = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, info->n*sizeof(cl_float4), V, &ret );

#pragma region CL Program
	// Priprava programa
	cl_program program;
	BuildKernel( &program, &context, &device_id, "kernelVecLocal.cl" );

	// priprava šcepca 
	krnl = clCreateKernel( program, "kernelVecLocal", &ret );
	ret |= clSetKernelArg( krnl, 2, sizeof(cl_mem), (void *) &devV );
	ret |= clSetKernelArg( krnl, 3, sizeof(cl_int), (void *) &(info->n) );
	ret |= clSetKernelArg( krnl, 4, sizeof(cl_float), (void *) &(info->eps) );
	ret |= clSetKernelArg( krnl, 5, sizeof(cl_float), (void *) &(info->kappa) );
	ret |= clSetKernelArg( krnl, 6, sizeof(cl_float), (void *) &(info->dt) );
	ret |= clSetKernelArg( krnl, 7, info->local_item_size * sizeof(cl_float4), NULL );

#pragma endregion


	glutMainLoop( );
}
