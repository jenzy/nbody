kernel void kernelCombo( global float4 *Coord,
							global float4 *newCoord,
							global float4 *V,
							int m,
							int n,
							float eps,
							float kappa,
							float dt,
							local float4* locCoord
							) {
	int id = get_global_id( 0 );
	int idGlobal = id + m;
	int idLocal = get_local_id( 0 );
	int blockSize = get_local_size( 0 );

	float4 myBody = Coord[idGlobal];
	float4 a = (float4) (0, 0, 0, 0);

	for( int j = 0; j < n; ) {
		locCoord[idLocal] = Coord[j + idLocal];
		barrier( CLK_LOCAL_MEM_FENCE );

		for( int k = 0; k < blockSize && j < n; k++, j++ ) {
			float4 dr = locCoord[k] - myBody;
			float4 dr2 = dr*dr;
			float invr = 1.f / sqrt( dr2.x + dr2.y + dr2.z + eps );

			float force = kappa * myBody.w * invr*invr*invr; // F = G * M / r^3
			a += force * dr;
		}
		barrier( CLK_LOCAL_MEM_FENCE );
	}
	a.w = 0.f;
	newCoord[idGlobal] = myBody + V[id] * dt + a*(dt*dt*0.5f);
	V[id] += a * dt;
}