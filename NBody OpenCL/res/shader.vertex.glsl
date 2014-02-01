
uniform float distCameraToCenter;
uniform float sphereRadius;

// relative distance from a particle to the center of the sphere, along the z axis, € [-1, 1]
varying float relDist;

void main()
{
	// apply the MV and P transformations
    vec4 cs_position = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * cs_position;

	// pass the color to fragment shader
	gl_FrontColor = gl_Color;

    float distCameraToParticle = -cs_position.z;			// distance from camera to particle
	float dr = distCameraToParticle - distCameraToCenter;	// distance from center to particle

	relDist = clamp(dr/sphereRadius, -1, 1);
}