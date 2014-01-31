/*
void main( ) {
	gl_Position = ftransform( );
}

*/

varying float distToCamera;

void main()
{
    vec4 cs_position = gl_ModelViewMatrix * gl_Vertex;
    distToCamera = -cs_position.z;
    gl_Position = gl_ProjectionMatrix * cs_position;
	gl_FrontColor = gl_Color;
}