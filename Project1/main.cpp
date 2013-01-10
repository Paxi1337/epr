#include "./includes/gameoflife.h"
#include "./includes/Timer.h"

int main(int argc, char** argv) {
	char* fInFName = 0;
	char* fOutFName = 0;
	char* fileToCompare = 0;
	int generations = 0;
	int nthreads = 1;
	bool measure = false;

	Timer t;

	for(int i=0;i<argc;++i) {
		
		// input file game field
		if(strcmp(argv[i], "--load") == 0) {
			if(argv[i+1]) {
				fInFName = argv[i+1];
			}
			else {
				MessageBoxA(0,"You specified no filename for --load", "ERROR", MB_OK);
				return -1;
			}
		}

		// output file game field
		else if(strcmp(argv[i], "--save") == 0) {
			if(argv[i+1]) {
				fOutFName = argv[i+1];
			}
			else {
				MessageBoxA(0,"You specified no filename for --save", "ERROR", MB_OK);
				return -1;
			}
		}

		// amount of generations to be calculated
		else if(strcmp(argv[i], "--generations") == 0) {
			if(argv[i+1]) {
				generations = atoi(argv[i+1]);
			}
			else {
				MessageBoxA(0,"You specified no count for --generations", "ERROR", MB_OK);
				return -1;
			}
		}

		else if(strcmp(argv[i], "--measure") == 0) {
			measure = true;
		}

		// check for mode to run
		else if(strcmp(argv[i], "--mode") == 0) {
			
			// OpenMP Mode selected
			if(strcmp(argv[i+1], "omp") == 0) {
				
				if(strcmp(argv[i+2], "--threads") == 0) {
					nthreads = atoi(argv[i+3]);

					if(nthreads < 1 || nthreads > 16) {
						MessageBoxA(0,"Threadnumber may not be bellow 1 or above 16", "ERROR", MB_OK);
						return -1;
					}

				}
			}
			if(strcmp(argv[i+1], "ocl") == 0) {
				OutputDebugStringA("OpenCL mode\n");
			}
			else if(strcmp(argv[i+1], "seq") == 0) {
				// nothing to do in here, the programs just runs with one thread
			}
		}

		// [optional]
		else if(strcmp(argv[i], "--fc") == 0) {
			if(argv[i+1]) {
				fileToCompare = argv[i+1];
			}
			else {
				MessageBoxA(0,"You specified no name for the file to compare", "ERROR", MB_OK);
				return -1;
			}
		}

	}

	if(!fInFName) {
		MessageBoxA(0,"You specified no input filename", "ERROR", MB_OK);
		return -1;
	}

	if(!fOutFName) {
		MessageBoxA(0,"You specified no output filename", "ERROR", MB_OK);
		return -1;
	}

	if(generations == 0) 
		generations = 250;



	t.start();
	Gameoflife<char>* gof = new Gameoflife<char>(fInFName);
	t.stop();



	gof->openCL_initPlatforms();
	gof->openCL_initDevices();

	gof->openCL_initContext();
	gof->openCL_initCommandQueue();
	gof->openCL_initMem();
	gof->openCL_initProgram();
	gof->openCL_initKernel();

	t.start();
	gof->openCL_run(250);
	t.stop();

	if(measure)
		std::cout << "OpenCLKernel execution time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;

	//if(measure)
	//	std::cout << "init time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;

	//t.start();
	//for(int i=0;i<generations;++i) {
	//	/*std::cout << gof;
	//	system("cls");
	//	*/
	//	gof->calcGenerationOpenMP();
	//}
	//t.stop();

	//if(measure)
	//	std::cout << "kernel time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;


	//if(fileToCompare) {
	//	if(gof->cmpFiles(fOutFName, fileToCompare))
	//		MessageBoxA(0,"Files are identical", "OK", MB_OK);
	//}

	t.start();
	gof->saveFile(fOutFName);
	delete gof;
	t.stop();
	
	if(measure)
		std::cout << "finalize time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;

	getchar();
	
}