varying float alpha;

void main()
{
	//float alpha = 1.0 -  alpha/50.0;
	gl_FragColor = vec4(gl_Color.rgb, alpha );
}