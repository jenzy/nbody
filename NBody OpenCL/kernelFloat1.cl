__kernel void kernelFloat1( __global float *X,
							__global float *Y,
							__global float *Z,
							__global float *newX,
							__global float *newY,
							__global float *newZ,
							__global float *VX,
							__global float *VY,
							__global float *VZ,
							__global float *M,
							int n,
							float eps,
							float kappa,
							float dt
						) 
{
	int id = get_global_id( 0 );

	if( id < n ) {
		int j;
		float ax=0, ay=0, az=0, dx, dy, dz, invr, force;
		float x = X[id], y = Y[id], z = Z[id];

		for( j = 0; j < n; j++ ) {
			dx = X[j] - x;
			dy = Y[j] - y;
			dz = Z[j] - z;

			invr = 1.0f / sqrt( dx*dx + dy*dy + dz*dz + eps );
			force = kappa * M[j] * invr*invr*invr;

			ax += force * dx;
			ay += force * dy;
			az += force * dz;
		}
		float dt2 = dt*dt*0.5f;
		newX[id] = x + VX[id] * dt + ax*dt2; // x = x + v*t + 0.5*a*t^2
		newY[id] = y + VY[id] * dt + ay*dt2;
		newZ[id] = z + VZ[id] * dt + az*dt2;

		VX[id] += ax * dt;
		VY[id] += ay * dt;
		VZ[id] += az * dt;
	}
}