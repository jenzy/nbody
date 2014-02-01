
uniform float distCameraToCenter;
uniform float sphereRadius;
varying float alpha;

void main()
{
	// apply the MV and P transformations
    vec4 cs_position = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * cs_position;

	// pass the color to fragment shader
	gl_FrontColor = gl_Color;

    float distCameraToParticle = -cs_position.z;			// distance from camera to particle
	float dr = distCameraToParticle - distCameraToCenter;	// distance from center to particle

	//float tmp = (dr / sphereRadius + 1.0 ) / 2.0;
	float tmp = (dr + sphereRadius) / ( 2.0 * sphereRadius );	// € (0, 1)
	alpha = 1.0 - clamp(tmp, 0.0, 1.0);
}