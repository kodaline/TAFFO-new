/*
 * fft.cpp
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 */

#include "tritri.hpp"

#include <fstream>
#include <iostream>
#include <map>

#include <time.h>       /* time */

#include "benchmark.hpp"

int main(int argc, char* argv[])
{
	int i;
	int x;
	int n;

	std::cout.precision(8);

	std::string inputFilename = argv[1];
	std::string outputFilename = argv[2];


	// prepare the output file for writting the theta values
	std::ofstream outputFileHandler;
	outputFileHandler.open(outputFilename);
	outputFileHandler.precision(5);

	// prepare the input file for reading the theta data
	std::ifstream inputFileHandler (inputFilename, std::ifstream::in);

	// first line defins the number of enteries
	inputFileHandler >> n;

	// create the directory for storing data
	float* __attribute((annotate("scalar(range('_MIN_XYZ_RANGE', '_MAX_XYZ_RANGE') error(1e-8)) backtracking(2)"))) xyz = (float*)malloc(n * 6 * 3 * sizeof(float)) ;
	if(xyz == NULL)
	{
		std::cout << "cannot allocate memory for the triangle coordinates!" << std::endl;
		return -1 ;
	}

	i = 0;
	while (i < n)
	{
		float __attribute((annotate("scalar(disabled range('_MIN_A_RANGE', '_MAX_A_RANGE'))"))) a[18];
		inputFileHandler >> 	a[0] 	>> 	a[1] 	>> 	a[2] 	>> a[3] 	>> a[4] 		>> a[5] >>
						a[6]	>>	a[7]	>> 	a[8]	>> a[9]		>> a[10]		>> a[11] >>
						a[12]	>> 	a[13] 	>> 	a[14] 	>> a[15] 	>> a[16]	 	>> a[17];
		for(int j = 0 ; j < 18; j++)
		{
			xyz[i * 18 + j] = a[j];
		}
		i++;
	}

	int totalCount = 0;
	int outputAggreg[6] = {0};

  uint64_t kernel_time = 0;
  AxBenchTimer timer;

	for(i = 0 ; i < (n * 6 * 3); i += 6 * 3)
	{

		int output = -1;

#pragma parrot(input, "jmeint", [18]dataIn)

		x = tri_tri_intersect(
				xyz + i + 0 * 3, xyz + i + 1 * 3, xyz + i + 2 * 3,
				xyz + i + 3 * 3, xyz + i + 4 * 3, xyz + i + 5 * 3,
				&output);

#pragma parrot(output, "jmeint", [2]<0.2; 0.8>dataOut)

		kernel_time += timer.nanosecondsSinceInit();

    outputAggreg[output] += 1;
    PRINT_INSTR("exit type = %d\n", output);
		outputFileHandler << x << " 0 0 " << output << std::endl;
		
		timer.reset();
	}
	
	kernel_time += timer.nanosecondsSinceInit();
	
	for (int i=0; i<6; i++)
	  PRINT_INSTR("exit type %d total = %d\n", i, outputAggreg[i]);
	
  std::cout << "kernel time = " << ((double)kernel_time) / 1000000000.0 << " s" << std::endl;

	outputFileHandler.close();
	inputFileHandler.close();

	free(xyz) ;
	xyz = NULL ;

	return 0 ;
}

extern "C"
fptype* newmain(inputDataStruct* &inputData){

	x = tri_tri_intersect(
	xyz + i + 0 * 3, xyz + i + 1 * 3, xyz + i + 2 * 3,
	xyz + i + 3 * 3, xyz + i + 4 * 3, xyz + i + 5 * 3,
	&output);

}
