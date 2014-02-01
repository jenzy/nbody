varying float alpha;

void main()
{
	gl_FragColor = vec4(gl_Color.rgb, alpha );
}