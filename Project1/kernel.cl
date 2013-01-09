__kernel
void calcGeneration(__global char* A,
					__global int* C) {

   int idx = get_global_id(0);

   C[idx] = A[idx];
   
}