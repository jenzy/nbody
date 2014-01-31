#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GL.h"
#include "Main.h"

bool GL::m_paused = false;
WOCL *GL::CL = nullptr;
info_t *GL::m_info;
GLuint GL::m_vboVertices[2];
cl_mem GL::devCoord[2];
int GL::idx = 0;
float GL::angleY = 45;
float GL::cameraDistance = 25.0f;

GLuint GL::m_shaderVert;
GLuint GL::m_shaderFrag;
GLuint GL::m_program;
GLint GL::m_uniformDistToCamera = 1337;

GL::GL( int width, int height, info_t *info ) {
	printf( "\n\n== OpenGL + OpenCL ==          N: %d\n", info->n );
	m_info = info;

#pragma region Create GLUT Window
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( width, height);
	glutInitWindowPosition( glutGet( GLUT_SCREEN_WIDTH ) / 2 - width/2, glutGet( GLUT_SCREEN_HEIGHT ) / 2 - height/2 );
	glutCreateWindow( "N-Body" );
	glewInit( );
#pragma endregion

#pragma region Register GLUT Callbacks
	glutDisplayFunc( GL::Display );
	glutTimerFunc( REFRESH_EVERY_X_MS, GL::Refresh, REFRESH_EVERY_X_MS );	//determin a minimum time between frames
	glutKeyboardFunc( Keyboard );
	glutSpecialFunc( KeyboardSpecial );
	//glutMouseFunc( appMouse );
	//glutMotionFunc( appMotion );
#pragma endregion

#pragma region Initialize CL
	CL = new WOCL( CL_DEVICE_TYPE_GPU, true );
	CL->SetWorkSize( info->local_item_size, WOCL::CalculateNumOfGroups( info->local_item_size, info->n ), 0 );
	CL->CreateAndBuildKernel( "kernelVec.cl", "kernelVec" );
#pragma endregion

}

GL::~GL() {
	if( CL != nullptr ) delete CL;
}

void GL::Init() {
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	//glDisable( GL_DEPTH_TEST );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_POINT_SMOOTH );
	glPointSize( 6. );
	glColor3f( 1.0f, 1.0f, 1.0f );

	glEnableClientState( GL_VERTEX_ARRAY );
	//glEnableClientState( GL_COLOR_ARRAY );
	//glDisableClientState( GL_NORMAL_ARRAY );

	m_shaderVert = CreateShader( GL_VERTEX_SHADER, "shader.vertex.glsl" );
	m_shaderFrag = CreateShader( GL_FRAGMENT_SHADER, "shader.fragment.glsl" );
	m_program = glCreateProgram( );
	glAttachShader( m_program, m_shaderVert );
	glAttachShader( m_program, m_shaderFrag );
	glLinkProgram( m_program );
	glUseProgram( m_program );

	m_uniformDistToCamera = glGetUniformLocation( m_program, "distCameraToCenter" );
	glUniform1f( m_uniformDistToCamera, cameraDistance );

#pragma region Projection Matrix
	int height = glutGet( GLUT_WINDOW_HEIGHT );
	int width = glutGet( GLUT_WINDOW_WIDTH );
	glViewport( 0, 0, width, width );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 90.0, (GLfloat) width / (GLfloat) width, 0.1, 1000.0 );
#pragma endregion

	// Host alokacija
	float *Coord = (float*) malloc( sizeof(cl_float4) * m_info->n );
	generateCoordinatesFloat4( Coord, m_info );
	float *V = (float *) calloc( m_info->n, 4 * sizeof(float) );

	// Device alokacija in kopiranje podatkov
	glGenBuffers( 2, m_vboVertices );
	glBindBuffer( GL_ARRAY_BUFFER, m_vboVertices[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cl_float4) * m_info->n, Coord, GL_DYNAMIC_DRAW ); // upload data to video card

	glBindBuffer( GL_ARRAY_BUFFER, m_vboVertices[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cl_float4) * m_info->n, Coord, GL_DYNAMIC_DRAW ); // upload data to video card

	devCoord[0] = CL->CreateBufferFromGLBuffer( CL_MEM_READ_WRITE, m_vboVertices[0] );
	devCoord[1] = CL->CreateBufferFromGLBuffer( CL_MEM_READ_WRITE, m_vboVertices[1] );
	cl_mem devV = CL->CreateBuffer( m_info->n*sizeof(cl_float4), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, V );

	free( Coord );
	free( V );

	//priprava šèepca
	CL->SetKernelArgument<cl_mem>( 2, &devV );
	CL->SetKernelArgument<cl_int>( 3, &(m_info->n) );
	CL->SetKernelArgument<cl_float>( 4, &(m_info->eps) );
	CL->SetKernelArgument<cl_float>( 5, &(m_info->kappa) );
	CL->SetKernelArgument<cl_float>( 6, &(m_info->dt) );
}

void GL::Play( ) {
	Init();

	glutMainLoop( );
}

void GL::Display( void ) {
	if( m_paused ) return;

	glFinish( );
	CL->AcquireObjectsFromGLAndFinish( 2, devCoord );

	//execute the kernel
	CL->SetKernelArgument<cl_mem>( 0, (devCoord+idx) );
	CL->SetKernelArgument<cl_mem>( 1, (devCoord+((idx+1)%2)) );
	CL->ExecuteKernel( );

	CL->ReleaseObjectsToGLAndFinish( 2, devCoord );

	idx = ++idx % 2;
	Render();
}

void GL::Render( void ) {
	if( m_paused ) return;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	UpdateView( );

	glBindBuffer( GL_ARRAY_BUFFER, m_vboVertices[idx] );
	glVertexPointer( 3, GL_FLOAT, sizeof(float), 0 );

	glDrawArrays( GL_POINTS, 0, m_info->n );

	glutSwapBuffers( );
}

void GL::Refresh( int ms ) {
	//this makes sure the Display function is called every ms miliseconds
	glutTimerFunc( ms, GL::Refresh, ms );
	glutPostRedisplay( );
}

void GL::Keyboard( unsigned char key, int x, int y ) {
	switch( key ) {
		case 27:		// ESC
			exit( 0 );
			break;
		case ' ':
			m_paused = !m_paused;
			break;
		case '+':
			cameraDistance -= DELTA_CAMERA_DISTANCE;
			if( m_uniformDistToCamera != 1337 )
				glUniform1f( m_uniformDistToCamera, cameraDistance );
			break;
		case '-':
			cameraDistance += DELTA_CAMERA_DISTANCE;
			if( m_uniformDistToCamera != 1337 )
				glUniform1f( m_uniformDistToCamera, cameraDistance );
			break;
		default:
			break;
	}
}

void GL::KeyboardSpecial( int key, int x, int y ) {
	switch( key ) {
		case GLUT_KEY_LEFT:
			angleY -= DELTA_ANGLE_Y;
			break;
		case GLUT_KEY_RIGHT:
			angleY += DELTA_ANGLE_Y;
			break;
		default:
			break;
	}
}

void GL::UpdateView() {
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glTranslatef( 0.0, 0.0, -cameraDistance );
	glRotatef( angleY, 0, 1, 0 );
	//glRotatef( 25, 1, 0, 0 );
}


void GL::CheckShaderCompileStatus( GLuint shader ) {
	int bufflen;
	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &bufflen );
	if( bufflen > 1 ) {
		GLchar* log_string = new char[bufflen + 1];
		glGetShaderInfoLog( shader, bufflen, 0, log_string );
		printf( "Log:\n%s", log_string );
		delete log_string;
	}

	int success;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
	if( success != GL_TRUE ) {
		printf( "Failed to compile shader.\n" );
		exit( -1001 );
	} 
}
GLint GL::CreateShader( GLenum shaderType, char *filename ) {
	GLint shader = glCreateShader( shaderType );
	const GLchar *src = WOCL::ReadWholeFile( filename, NULL );
	glShaderSource( shader, 1, &src, NULL );
	glCompileShader( shader );
	CheckShaderCompileStatus( shader );
	return shader;
}
