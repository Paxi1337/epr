#ifndef __GAMEOFLIFE_H
#define __GAMEOFLIFE_H

#include <Windows.h>
#include <fstream>
#include <cassert>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <CL/cl.h>

char* readSource(const char *sourceFilename);

template <class T>
class Gameoflife {
public:
	Gameoflife();
	explicit Gameoflife(const char* fileName);
	~Gameoflife();

	bool loadFile(const char* fileName);
	bool saveFile(const char* fileName);
	bool cmpFiles(const char* fileName1, const char* fileName2) const;
	void calcGeneration(void);

	// openMP 
	bool loadFileOpenMP(const char* fileName);
	void calcGenerationOpenMP(void);

	void openCL_initPlatforms();
	void openCL_initDevices();
	void openCL_initContext();
	void openCL_initCommandQueue(const int deviceID);
	void openCL_initMem();
	void openCL_initProgram();
	void openCL_initKernel();
	void openCL_run();
	
	// std::ostream can use private array of gof
	friend std::ostream& operator<<(std::ostream& os, const Gameoflife<T>& gof);
private:
	std::ifstream mInputFile;
	std::fstream mOutputFile;
	// contiguous chunk of memory that holds the data
	T* mData;
	// copy of mData
	T* mDataTmp;
	// stores the pointer for each row of the data
	// used for easier indexing afterwards mIndexArray[row][col]
	T** mIndexArray;

	// x/y dim of field
	int mXDim;
	int mYDim;


	//OPENCL specific code

	// platforms
	cl_uint mNumPlatforms;
    cl_platform_id* mPlatforms;
	
	// devices
	cl_uint mNumDevices;
    cl_device_id* mDevices;

	// context
	cl_context mContext;

	// command queue to selected device
	cl_command_queue mCmdQueue; 

	// input and output memory
	cl_mem mMemIn;
	cl_mem mMemOut;

	// source code of kernel
	cl_program mProgram;

	// kernel running on the GPU
	cl_kernel mKernel;

};

template <class T>
Gameoflife<T>::Gameoflife() : mData(0), mDataTmp(0), mIndexArray(0), mXDim(0), mYDim(0),
						      mNumPlatforms(0), mPlatforms(0),
							  mNumDevices(0), mDevices(0)
							  
{

}

template <class T>
Gameoflife<T>::Gameoflife(const char* fileName) : mData(0), mDataTmp(0), mIndexArray(0), mXDim(0), mYDim(0),
											      mNumPlatforms(0), mPlatforms(0),
												  mNumDevices(0), mDevices(0),
												  mContext(0), mCmdQueue(0)

{
	loadFile(fileName);
}

template <class T>
Gameoflife<T>::~Gameoflife() {
	if(mData)
		delete[] mData;
	if(mDataTmp) 
		delete[] mDataTmp;
	if(mIndexArray)
		delete[] mIndexArray;

	// dont forget to free OpenCL data
}

template <class T>
bool Gameoflife<T>::loadFile(const char* fileName) {
	// open input file
	mInputFile.open(fileName, std::ifstream::in);
	if(!mInputFile.is_open()) {
		MessageBoxA(0,"Could not load input file","ERROR", MB_OK);
		return false;
	}

	std::string line;
	// get first line for information about x and y dim
	std::getline(mInputFile,line);

	{
		std::stringstream ss;
		size_t pos = line.find_first_of(',');

		// store x dimension
		ss.str(line.substr(0,pos));
		ss >> mXDim;

		// reset stringstream
		ss.str("");
		ss.clear();

		// store y dimension
		ss.str(line.substr(pos+1));
		ss >> mYDim;
	}

	int offset = 0;
	int row = 0;

	// alloc one more byte of memory for the 0 byte at the end of the last line
	mData = new T[mXDim*mYDim+1];
	// allocating array for mYDim char*´s
	mIndexArray = new T*[mYDim];

	while(std::getline(mInputFile,line)){
		// currently not saving 0 byte use c_str()+1 instead if needed and change allocated amount of memory to myDimX+1 instead of myDimX
		//strcpy(mData+offset,line.c_str());
		memcpy(mData+offset,line.c_str(),line.length());
		// storing the pointer to the line in mData array just copied
		mIndexArray[row] = mData+offset;

		//printf("%p\n",mIndexArray[row]);

		offset = offset+mXDim;
		row++;
	}
	
	mDataTmp = new T[mXDim*mYDim+1];
	memcpy(mDataTmp,mData,mXDim*mYDim+1);

	// last line seems to end with a 0 byte anyway in input files
	//mIndexArray[mYDim][mXDim-1] = '\0';

	return true;
}

template <class T>
bool Gameoflife<T>::loadFileOpenMP(const char* fileName) {
	// open input file
	mInputFile.open(fileName, std::ifstream::in);
	if(!mInputFile.is_open()) {
		MessageBoxA(0,"Could not load input file","ERROR", MB_OK);
		return false;
	}

	std::string line;
	// get first line for information about x and y dim
	std::getline(mInputFile,line);

	{
		std::stringstream ss;
		size_t pos = line.find_first_of(',');

		// store x dimension
		ss.str(line.substr(0,pos));
		ss >> mXDim;

		// reset stringstream
		ss.str("");
		ss.clear();

		// store y dimension
		ss.str(line.substr(pos+1));
		ss >> mYDim;
	}

	int offset = 0;
	int row = 0;

	// alloc one more byte of memory for the 0 byte at the end of the last line
	mData = new T[mXDim*mYDim+1];
	// allocating array for mYDim char*´s
	mIndexArray = new T*[mYDim];
	#pragma omp parallel
	{
		while(std::getline(mInputFile,line)){
			// currently not saving 0 byte use c_str()+1 instead if needed and change allocated amount of memory to myDimX+1 instead of myDimX
			//strcpy(mData+offset,line.c_str());
			memcpy(mData+offset,line.c_str(),line.length());
			// storing the pointer to the line in mData array just copied
			mIndexArray[row] = mData+offset;

			//printf("%p\n",mIndexArray[row]);

			offset = offset+mXDim;
			row++;
		}
	}

	mDataTmp = new T[mXDim*mYDim+1];
	memcpy(mDataTmp,mData,mXDim*mYDim+1);

	// last line seems to end with a 0 byte anyway in input files
	//mIndexArray[mYDim][mXDim-1] = '\0';

	return true;
}

template <class T>
void Gameoflife<T>::calcGeneration() {
	for(int y=0;y<mYDim;++y) {
		for(int x=0;x<mXDim;++x){
			int neighbors = 0;
			// axes are notated as [y][x] since this is the layout of the indexData array
			// x = size of array x; y = size of array y
			
			int xLeft = x-1;
			int xRight = x+1;
			int yTop = y-1;
			int yBot = y+1;

			if(x==0) {
				xLeft = (x-1+mXDim)%mXDim;
			}

			if(x==mXDim-1) {
				xRight = (x+1+mXDim)%mXDim;
			}

			if(y==0) {
				yTop = (y-1+mYDim)%mYDim;
			}

			if(y==mYDim-1) {
				yBot = (y+1+mYDim)%mYDim;
			}

			// in case of accessing element on [-1][-1]
			if(mIndexArray[yTop][xLeft] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [-1][0]
			if(mIndexArray[yTop][x] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [-1][x]
			if(mIndexArray[yTop][xRight] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [0][-1]
			if(mIndexArray[y][xLeft] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [0][x]
			if(mIndexArray[y][xRight] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [y][-1]
			if(mIndexArray[yBot][xLeft] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [y][0]
			if(mIndexArray[yBot][x] == 'x') {
				neighbors++;
			}

			// in case of accessing element on [y][x]
			if(mIndexArray[yBot][xRight] == 'x') {
				neighbors++;
			}
			if(mIndexArray[y][x] == 'x') {
				if(neighbors > 3 || neighbors < 2) {
					mDataTmp[x+(y*mXDim)]= '.';
				}
			}
			else {
				if(neighbors == 3) {
					mDataTmp[x+(y*mXDim)]= 'x';
				}
			}
		}
	}
	memcpy(mData,mDataTmp,mXDim*mYDim+1);
}

template <class T>
void Gameoflife<T>::calcGenerationOpenMP() {
	#pragma omp parallel 
	{
		#pragma omp for

		for(int y=0;y<mYDim;++y) {
			for(int x=0;x<mXDim;++x){
				int neighbors = 0;
				// axes are notated as [y][x] since this is the layout of the indexData array
				// x = size of array x; y = size of array y
			
				int xLeft = x-1;
				int xRight = x+1;
				int yTop = y-1;
				int yBot = y+1;

				if(x==0) {
					xLeft = (x-1+mXDim)%mXDim;
				}

				if(x==mXDim-1) {
					xRight = (x+1+mXDim)%mXDim;
				}

				if(y==0) {
					yTop = (y-1+mYDim)%mYDim;
				}

				if(y==mYDim-1) {
					yBot = (y+1+mYDim)%mYDim;
				}

				// in case of accessing element on [-1][-1]
				if(mIndexArray[yTop][xLeft] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [-1][0]
				if(mIndexArray[yTop][x] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [-1][x]
				if(mIndexArray[yTop][xRight] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [0][-1]
				if(mIndexArray[y][xLeft] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [0][x]
				if(mIndexArray[y][xRight] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [y][-1]
				if(mIndexArray[yBot][xLeft] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [y][0]
				if(mIndexArray[yBot][x] == 'x') {
					neighbors++;
				}

				// in case of accessing element on [y][x]
				if(mIndexArray[yBot][xRight] == 'x') {
					neighbors++;
				}
				if(mIndexArray[y][x] == 'x') {
					if(neighbors > 3 || neighbors < 2) {
						mDataTmp[x+(y*mXDim)]= '.';
					}
				}
				else {
					if(neighbors == 3) {
						mDataTmp[x+(y*mXDim)]= 'x';
					}
				}
			}
		}
	} // parallel section end 
	memcpy(mData,mDataTmp,mXDim*mYDim+1);
}

template <class T>
bool Gameoflife<T>::saveFile(const char* fileName) {
	mOutputFile.open(fileName, std::ios::out);

	int l=0;

	// getting stringlength for writing width and height back to the file
	// it is assumed that the y dim varies between 100 and 99999 
	if(mYDim > 9999)
		l=l+5;
	else if(mYDim > 999)
		l=l+4;
	else // (100-999)
		l=l+3;

	// + xDim
	l=l+4;

	// +2 for , and \n char
	char* buffer = new char[l+2];
	int row = 0;
	_snprintf(buffer,l+2,"%d,%d\n",mXDim,mYDim);
	mOutputFile.write(buffer,l+2);
	delete[] buffer;

	for(int y=0;y<mYDim;++y) {
		for(int x=0;x<mXDim;++x){
			mOutputFile.put(mIndexArray[y][x]);
		}

		mOutputFile.put('\n');
	}

	mOutputFile.close();

	return true;
}

template <class T>
bool Gameoflife<T>::cmpFiles(const char* fileName1, const char* fileName2) const {

	std::string s1;
	std::string s2;

	std::ifstream a(fileName1,std::ifstream::in);
	std::ifstream b(fileName2,std::ifstream::in);

	if(!a.is_open() || !b.is_open()) {
		MessageBoxA(NULL, "Error opening compare file","ERROR", MB_OK);
		return false;
	}

	int row = 0;

	while(std::getline(a,s1)) {
		std::getline(b,s2);
		if(s1!=s2) {
			MessageBoxA(NULL, "Error comparing files","ERROR", MB_OK);
			return false;
		}
		row++;
	}

	return true;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Gameoflife<T>& gol) {
	for(int y=0;y<gol.mYDim;++y) {
		for(int x=0;x<gol.mXDim;++x){
			os << gol.mIndexArray[y][x];	
		}
		//os << gol.mIndexArray[y] << std::endl;
		//os << std::endl;
	}
	return os;
}

template <class T>
void Gameoflife<T>::openCL_initPlatforms() {
	cl_int status;  // use as return value for most OpenCL functions

    // Query for the number of recongnized platforms
    status = clGetPlatformIDs(0, NULL, &mNumPlatforms);
    if(status != CL_SUCCESS) {
       printf("clGetPlatformIDs failed\n");
       exit(-1);
    }

    // Make sure some platforms were found 
    if(mNumPlatforms == 0) {
       printf("No platforms detected.\n");
       exit(-1);
    }

    // Allocate enough space for each platform
    mPlatforms = (cl_platform_id*)malloc(mNumPlatforms*sizeof(cl_platform_id));
	//mPlatforms = new cl_platform_id[mNumPlatforms];
    if(mPlatforms == NULL) {
       perror("malloc");
       exit(-1);
    }

    // Fill in platforms
    clGetPlatformIDs(mNumPlatforms, mPlatforms, NULL);
    if(status != CL_SUCCESS) {
       printf("clGetPlatformIDs failed\n");
       exit(-1);
    }

    // Print out some basic information about each platform
    printf("%u platforms detected\n", mNumPlatforms);
    for(unsigned int i = 0; i < mNumPlatforms; i++) {
		char buf[100];
        printf("Platform %u: \n", i);
        status = clGetPlatformInfo(mPlatforms[i], CL_PLATFORM_VENDOR,
                       sizeof(buf), buf, NULL);
        printf("\tVendor: %s\n", buf);
        status |= clGetPlatformInfo(mPlatforms[i], CL_PLATFORM_NAME,
                       sizeof(buf), buf, NULL);
        printf("\tName: %s\n", buf);

        if(status != CL_SUCCESS) {
			printf("clGetPlatformInfo failed\n");
			exit(-1);
        }
    }
    printf("\n");
}

template <class T>
void Gameoflife<T>::openCL_initDevices() {
	cl_int status;  // use as return value for most OpenCL functions
	cl_uint numDevices = 0;
	int devicesPerPlatform[] = {0,0,0,0,0};

	for(unsigned int i = 0; i < mNumPlatforms; ++i) {
		status = clGetDeviceIDs(mPlatforms[i], CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, 0, NULL, 
                           &numDevices);
		
		if(status != CL_SUCCESS) {
			printf("clGetDeviceIDs failed\n");
			//exit(-1);
			
		}
		else {
			devicesPerPlatform[i] += numDevices;
			mNumDevices += numDevices;
		}
		
	}

    // Make sure some devices were found
    if(mNumDevices == 0) {
       printf("No devices detected.\n");
       //exit(-1);
    }
	else {
		printf("%d\n", mNumDevices);
	}
	
    // Allocate enough space for each device
    mDevices = (cl_device_id*)malloc(mNumDevices*sizeof(cl_device_id));
    if(mDevices == NULL) {
       perror("malloc");
       exit(-1);
    }

	unsigned int count = 0;

	for(unsigned int i = 0; i < mNumPlatforms; ++i) {
		for(unsigned int k = 0; k < devicesPerPlatform[i]; ++k) {
			
			
			/*if(k == 0)
				status = clGetDeviceIDs(mPlatforms[i], CL_DEVICE_TYPE_GPU, devicesPerPlatform.at(i), &mDevices[count], NULL);
			else
				status = clGetDeviceIDs(mPlatforms[i], CL_DEVICE_TYPE_CPU, devicesPerPlatform.at(i), &mDevices[count], NULL);*/
			
			// CL_DEVICE_TYPE_ALL finds CPU twice on laptop... dont no why so far
			status = clGetDeviceIDs(mPlatforms[i], CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, devicesPerPlatform[i], &mDevices[count], NULL);
			
			if(status != CL_SUCCESS) {
				printf("clGetDeviceIDs failed\n");
				//exit(-1);
			}

			count++;
		}
		
		
	}

    // Print out some basic information about each device
    printf("%u devices detected\n", mNumDevices);
    for(unsigned int i = 0; i < mNumDevices; i++) {
       char buf[150];
	   cl_uint numberBuf = 0;
	   cl_device_type deviceType = 0;

       printf("Device %u: \n", i);
       
	   status = clGetDeviceInfo(mDevices[i], CL_DEVICE_VENDOR,
                        sizeof(buf), buf, NULL);
	   printf("\tDevice: %s\n", buf);
       
	   status |= clGetDeviceInfo(mDevices[i], CL_DEVICE_NAME,
                        sizeof(buf), buf, NULL);
       printf("\tName: %s\n", buf);

	   status = clGetDeviceInfo(mDevices[i], CL_DEVICE_TYPE,
                        sizeof(deviceType), &deviceType, NULL);
       printf("\tDevice Type: %d\n", deviceType);

	   status |= clGetDeviceInfo(mDevices[i], CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(numberBuf), &numberBuf, NULL);
       printf("\tMax compute units: %d\n", numberBuf);

       if(status != CL_SUCCESS) {
          printf("clGetDeviceInfo failed\n");
          //exit(-1);
		  __debugbreak();
       }
    }
    printf("\n");
}


template <class T>
void Gameoflife<T>::openCL_initContext() {
	cl_int status;

	//// Create a context and associate it with the devices
	mContext = clCreateContext(NULL, 1, &mDevices[0], NULL, NULL, &status);
	//mContext = clCreateContext(NULL, 1, &mDevices[1], NULL, NULL, &status);
    if(status != CL_SUCCESS || mContext == NULL) {
       printf("clCreateContext failed\n");
       //exit(-1);
	   __debugbreak();
    }
	printf("created context to device 1 (Desktop: GTX680 / Ultrabook: GTX650M)\n");

}

template <class T>
void Gameoflife<T>::openCL_initCommandQueue(const int deviceID) {
	cl_int status;

	if(deviceID < 0 || deviceID > (mNumDevices - 1)) {
		__debugbreak();
	}
	
	mCmdQueue = clCreateCommandQueue(mContext, mDevices[deviceID], 0, &status);

	if(status != CL_SUCCESS || mCmdQueue == NULL) {
      printf("clCreateCommandQueue failed\n");
      exit(-1);
   }
}

template <class T>
void Gameoflife<T>::openCL_initMem() {
	cl_int status;

	// Create a buffer object (d_B) that contains the data from the host ptr B
	mMemIn = clCreateBuffer(mContext, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
		mXDim*mYDim+1*sizeof(T), mData, &status);
   if(status != CL_SUCCESS || mMemIn == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }

   // Create a buffer object (d_C) with enough space to hold the output data
   mMemOut = clCreateBuffer(mContext, CL_MEM_READ_WRITE, 
                   mXDim*mYDim+1*sizeof(T), NULL, &status);
   if(status != CL_SUCCESS || mMemOut == NULL) {
      printf("clCreateBuffer failed\n");
      exit(-1);
   }
}

template <class T>
void Gameoflife<T>::openCL_initProgram() {
	cl_int status;

	char* kernelCode;
	const char* kernelFileName = "kernel.cl";
	kernelCode = readSource(kernelFileName);

	mProgram = clCreateProgramWithSource(mContext, 1, (const char**)&kernelCode, 
                              NULL, &status);

    if(status != CL_SUCCESS) {
       printf("clCreateProgramWithSource failed\n");
       //exit(-1);
	   __debugbreak();
    }

	cl_int buildErr;
    // Build (compile & link) the program for the devices.
    // Save the return value in 'buildErr' (the following 
    // code will print any compilation errors to the screen)
    buildErr = clBuildProgram(mProgram, 1, mDevices, NULL, NULL, NULL);

    // If there are build errors, print them to the screen
    if(buildErr != CL_SUCCESS) {
       printf("Program failed to build.\n");
       cl_build_status buildStatus;
       for(unsigned int i = 0; i < 1; i++) {

		  // check if compiling and linking the program went fine
          clGetProgramBuildInfo(mProgram, mDevices[i], CL_PROGRAM_BUILD_STATUS,
                           sizeof(cl_build_status), &buildStatus, NULL);
          if(buildStatus == CL_SUCCESS) {
             continue;
          }

          char *buildLog;
          size_t buildLogSize;
		  // get size of build log
          clGetProgramBuildInfo(mProgram, mDevices[i], CL_PROGRAM_BUILD_LOG,
                           0, NULL, &buildLogSize);
          buildLog = (char*)malloc(buildLogSize);
          if(buildLog == NULL) {
             perror("malloc");
             //exit(-1);
			 __debugbreak();
          }
		  // create buildlog
          clGetProgramBuildInfo(mProgram, mDevices[i], CL_PROGRAM_BUILD_LOG,
                           buildLogSize, buildLog, NULL);
          buildLog[buildLogSize-1] = '\0';
          printf("Device %u Build Log:\n%s\n", i, buildLog);   
          free(buildLog);
       }
       //exit(0);
	   __debugbreak();
    }
	else {
		printf("No build errors\n");
	}
}

template <class T>
void Gameoflife<T>::openCL_initKernel() {
	cl_int status;

	// kernel function is calcGeneration
    mKernel = clCreateKernel(mProgram, "calcGeneration", &status);
    if(status != CL_SUCCESS) {
       printf("clCreateKernel failed\n");
       exit(-1);
    }

    // Associate the input and output buffers with the kernel 
	status = clSetKernelArg(mKernel, 0, sizeof(int), &mXDim);
	status = clSetKernelArg(mKernel, 1, sizeof(int), &mYDim);
	status |= clSetKernelArg(mKernel, 2, sizeof(cl_mem), &mMemIn);
    status |= clSetKernelArg(mKernel, 3, sizeof(cl_mem), &mMemOut);
    if(status != CL_SUCCESS) {
       printf("clSetKernelArg failed\n");
       exit(-1);
    }
}

template <class T>
void Gameoflife<T>::openCL_run() {
	cl_int status;

	// Define an index space (global work size) of threads for execution.  
    // A workgroup size (local work size) is not required, but can be used.
	size_t globalWorkSize[2] = {mYDim, mXDim};

    // Execute the kernel
	status = clEnqueueNDRangeKernel(mCmdQueue, mKernel, 2, NULL, globalWorkSize, 
                           NULL, 0, NULL, NULL);
    if(status != CL_SUCCESS) {
       printf("clEnqueueNDRangeKernel failed\n");
       exit(-1);
    }

 //   // Read the OpenCL output buffer (d_C) to the host output array (C)
	//clEnqueueReadBuffer(mCmdQueue, mMemOut, CL_TRUE, 0, mXDim*mYDim+1*sizeof(T), mData, 
 //                 0, NULL, NULL);
}


char* readSource(const char *sourceFilename) {

   FILE *fp;
   int err;
   int size;

   char *source;

   fp = fopen(sourceFilename, "rb");
   if(fp == NULL) {
      printf("Could not open kernel file: %s\n", sourceFilename);
      exit(-1);
   }
   
   err = fseek(fp, 0, SEEK_END);
   if(err != 0) {
      printf("Error seeking to end of file\n");
      exit(-1);
   }

   size = ftell(fp);
   if(size < 0) {
      printf("Error getting file position\n");
      exit(-1);
   }

   err = fseek(fp, 0, SEEK_SET);
   if(err != 0) {
      printf("Error seeking to start of file\n");
      exit(-1);
   }

   source = (char*)malloc(size+1);
   if(source == NULL) {
      printf("Error allocating %d bytes for the program source\n", size+1);
      exit(-1);
   }

   err = fread(source, 1, size, fp);
   if(err != size) {
      printf("only read %d bytes\n", err);
      exit(0);
   }

   source[size] = '\0';

   return source;
}

#endif


