#pragma once
#include "WOCL.h"
#include <CL\cl.hpp>
#include <GL/freeglut.h>
#include "Main.h"

#define REFRESH_EVERY_X_MS 30
#define DELTA_ANGLE_Y 1
#define DELTA_CAMERA_DISTANCE 5

class GL {
public:
	GL( int width, int height, info_t *info );
	~GL();

	void Play();
	void Init();

private:
	static bool m_paused;
	static WOCL *CL;
	static info_t *m_info;
	static GLuint m_vboVertices[2];
	static cl_mem devCoord[2];
	static int idx;
	static float angleY;
	static float cameraDistance;

	static GLuint m_shaderVert;
	static GLuint m_shaderFrag;
	static GLuint m_program;
	static GLint m_uniformDistToCamera;

	static void Display( void );
	static void Render( void );
	static void Refresh( int ms );
	static void Keyboard( unsigned char key, int x, int y );
	static void KeyboardSpecial( int key, int x, int y );
	static void UpdateView();

	static void CheckShaderCompileStatus( GLuint shader );
	static GLint CreateShader( GLenum shaderType, char *filename );
};

