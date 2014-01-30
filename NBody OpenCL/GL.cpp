#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GL.h"


GL::GL( size_t width, size_t height ) {
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( width, height);
	glutInitWindowPosition( glutGet( GLUT_SCREEN_WIDTH ) / 2 - width/2, glutGet( GLUT_SCREEN_HEIGHT ) / 2 - height/2 );
	glutCreateWindow( "N-Body" );

	//glutDisplayFunc( render ); //main rendering function
	//glutTimerFunc( 30, timerCB, 30 ); //determin a minimum time between frames
	//glutKeyboardFunc( appKeyboard );
	//glutMouseFunc( appMouse );
	//glutMotionFunc( appMotion );

	glewInit( );

	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glDisable( GL_DEPTH_TEST );

	// projection
	glViewport( 0, 0, width, width );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 90.0, (GLfloat) width / (GLfloat) width, 0.1, 1000.0 );

	// view matrix
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glTranslatef( 0.0, 0.0, -25.0f );
}


GL::~GL() {}
