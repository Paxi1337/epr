__kernel
void calcGeneration(__global int *A,
					__global int *C) {

   int idx = get_global_id(0);

   C[idx] = A[idx];
   
}
