#pragma once
#include "WOCL.h"
#include <CL\cl.hpp>
#include <GL\freeglut.h>
#include "Main.h"
#include "Timer.h"

#define REFRESH_EVERY_X_MS 30
#define DELTA_ANGLE_Y 1
#define DELTA_ANGLE_X 1
#define DELTA_CAMERA_DISTANCE 5

class GL {
public:
	GL( int width, int height, info_t *info );
	~GL();

	void Start();
	void Init();

private:
	static bool m_paused;
	static float m_angleY, m_angleX, m_cameraDistance;
	
	static WOCL *CL;
	static Timer m_timer;
	static info_t *m_info;

	static GLuint m_vboVertices[2];
	static cl_mem devCoord[2];
	static int idx;

	static GLint m_uniformDistToCamera;
	static GLint m_uniformSphereRadius;

	static void Display( void );	// The "update" function
	static void Render( void );		// The "render" function
	static void Refresh( int ms );
	static void UpdateView();		// Updates the view matrix
	static void Keyboard( unsigned char key, int x, int y );
	static void KeyboardSpecial( int key, int x, int y );

	static void CheckShaderCompileStatus( GLuint shader );
	static GLint CreateShader( GLenum shaderType, char *filename );
};

