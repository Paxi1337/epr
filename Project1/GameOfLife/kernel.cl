__kernel
void calcGeneration(int xDim, int yDim, __global char* in, __global char* out) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int left = x - 1;
	int right = x + 1;
	int top = y - 1;
	int bot = y + 1;

	int neighbors = 0;
	
	if(x == 0) {
		left = (x-1+xDim)%xDim;
	}

	if(x == xDim-1) {
		right = (x+1)%xDim;
	}

	if(y == 0) {
		top = (y-1+yDim)%yDim;
	}

	if(y == yDim-1) {
		bot = (y+1)%yDim;
	}

	// leftupper
	if(in[left + top * xDim] == 'x') {
		neighbors++;
	}

	// upper
	if(in[x + top * xDim] == 'x') {
		neighbors++;
	}

	// right upper
	if(in[right + top * xDim] == 'x') {
		neighbors++;
	}

	// right
	if(in[right + y * xDim] == 'x') {
		neighbors++;
	}

	// right bottom
	if(in[right + bot * xDim] == 'x') {
		neighbors++;
	}

	// bottom
	if(in[x + bot * xDim] == 'x') {
		neighbors++;
	}

	// left bottom
	if(in[left + bot * xDim] == 'x') {
		neighbors++;
	}

	// left
	if(in[left + y * xDim] == 'x') {
		neighbors++;
	}

	if(in[x + y * xDim] == 'x') {
		if(neighbors > 3 || neighbors < 2) {
			out[x + y * xDim]= '.';
		}
	}
	else {
		if(neighbors == 3) {
			out[x + y * xDim] = 'x';
		}
	}
}