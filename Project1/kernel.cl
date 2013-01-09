__kernel
void calcGeneration(const int xDim, const int yDim, __global char* in, __global char* out) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int left = x - 1;
	int right = x + 1;
	int top = y - 1;
	int bot = y + 1;
	
	 

   
}