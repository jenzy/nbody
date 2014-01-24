kernel void kernelCombo( global float4 *Coord,
							global float4 *newCoord,
							global float4 *V,
							int m,
							int n,
							float eps,
							float kappa,
							float dt
							) {
	int id = get_global_id( 0 );
	int idGlobal = id + m;

	if( id < n ) {
		float4 myBody = Coord[idGlobal];
		float4 a = (float4) (0, 0, 0, 0);

		for( int j = 0; j < n; j++ ) {
			float4 dr = Coord[j] - myBody;
			float4 dr2 = dr*dr;
			float invr = 1.f / sqrt( dr2.x + dr2.y + dr2.z + eps );

			float force = kappa * myBody.w * invr*invr*invr; // F = G * M / r^3
			a += force * dr;
		}
		a.w = 0.f;
		newCoord[id] = myBody + V[id] * dt + a*(dt*dt*0.5f);
		V[id] += a * dt;
	}
}