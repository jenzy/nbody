__kernel void vector_add(	__global const double *A,		
							__global const double *B,		
							__global double *C,				
							int size)						
{														
	int i = get_global_id(0);							
	while( i < size )									
	{													
		C[i] = A[i] + B[i];								
		i += get_global_size(0);	
	}													
}														