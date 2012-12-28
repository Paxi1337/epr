#include "./includes/gameoflife.h"
#include "./includes/Timer.h"

bool useOpenMP;

int main(int argc, char** argv) {
	char* fInFName = 0;
	char* fOutFName = 0;
	char* fileToCompare = 0;
	int generations = 0;
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

		else if(strcmp(argv[i], "--mode") == 0) {
			if(strcmp(argv[i+1], "omp") == 0) {
				useOpenMP = true;
				OutputDebugStringA("OpenMP mode\n");
			}
			if(strcmp(argv[i+1], "ocl") == 0) {
				useOpenMP = true;
				OutputDebugStringA("OpenCL mode\n");
			}
			else if(strcmp(argv[i+1], "seq") == 0) {
				useOpenMP = false;
				OutputDebugStringA("Seq mode\n");
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

	if(measure)
		std::cout << "init time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;

	t.start();
	for(int i=0;i<generations;++i) {
		/*std::cout << gof;
		system("cls");
		*/
		gof->calcGeneration();
	}
	t.stop();

	if(measure)
		std::cout << "kernel time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;


	if(fileToCompare) {
		if(gof->cmpFiles(fOutFName, fileToCompare))
			MessageBoxA(0,"Files are identical", "OK", MB_OK);
	}

	t.start();
	gof->saveFile(fOutFName);
	delete gof;
	t.stop();
	
	if(measure)
		std::cout << "finalize time in seconds " << t.getElapsedTimeInSec() << ";" << std::endl;

	getchar();
	
}