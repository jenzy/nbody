/*
void main( ) {
	gl_Position = ftransform( );
}

*/

uniform float distCameraToCenter;
varying float alpha;

void main()
{
    vec4 cs_position = gl_ModelViewMatrix * gl_Vertex;
    float distToCamera = -cs_position.z;
    gl_Position = gl_ProjectionMatrix * cs_position;

	gl_FrontColor = gl_Color;
	//alpha = distToCamera;
	alpha = 1.0 - distToCamera/(distCameraToCenter+10);
}