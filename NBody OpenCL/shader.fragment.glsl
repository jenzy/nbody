
// relative distance from a particle to the center of the sphere, along the z axis, € [-1, 1]
varying float relDist;	 

void main()
{
	float alpha = 1 - (( relDist + 1 ) / 2);
	float color = abs(relDist);
	gl_FragColor = vec4(color, color, 1, alpha );
}