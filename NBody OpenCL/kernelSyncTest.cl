#define SWAP(a,b) do {__global float *temp=a; a=b; b=temp;} while(0)

__kernel void kernelSyncTest( __global float *X,
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
							float dt,
							float steps
							) {
	int id = get_global_id( 0 );

	if( id < n ) {
		for( int i = 0; i < steps; i++ ) {

			int j;
			float ax = 0, ay = 0, az = 0, dx, dy, dz, invr, force;
			float x = X[id];
			float y = Y[id];
			float z = Z[id];

			for( j = 0; j < n; j++ ) {
				dx = X[j] - x;
				dy = Y[j] - y;
				dz = Z[j] - z;

				//Izvedemo trik brez uporabe if stavkov
				invr = 1.0f / sqrt( dx*dx + dy*dy + dz*dz + eps );
				force = kappa * M[j] * invr*invr*invr;

				ax += force * dx; // izracun skupnega pospeska
				ay += force * dy;
				az += force * dz;
			}
			float dt2 = dt*dt;
			newX[id] = x + VX[id] * dt + 0.5f*ax*dt2; // nov polozaj za telo i
			newY[id] = y + VY[id] * dt + 0.5f*ay*dt2;
			newZ[id] = z + VZ[id] * dt + 0.5f*az*dt2;

			VX[id] += ax * dt; /* nova hitrost za telo i */
			VY[id] += ay * dt;
			VZ[id] += az * dt;

			barrier( CLK_GLOBAL_MEM_FENCE );
			SWAP( newX, X );
			SWAP( newY, Y );
			SWAP( newZ, Z );
		}
	}
}