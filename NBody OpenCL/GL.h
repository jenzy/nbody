#pragma once
#include "WOCL.h"
#include <CL\cl.hpp>
#include <GL/freeglut.h>
#include "Main.h"

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
	static GLuint m_vboVertices;
	static cl_mem devCoord;
	static cl_mem devCoordNew;


	static void Display( void );
	static void Refresh( int ms );
	static void Keyboard( unsigned char key, int x, int y );
};

