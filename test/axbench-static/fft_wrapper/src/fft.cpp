#include <cstdio>
#include <iostream>
#include "fourier.hpp"
#include <fstream>
#include <time.h>
#include "benchmark.hpp"

typedef struct data_ {
    int* K;
    int* indices;
    Complex* x;
    Complex* f;
} data;


static int* indices;
static Complex* __attribute((annotate("target('x') " ANNOTATION_COMPLEX_RANGE))) x;
static Complex* __attribute((annotate("target('f') " ANNOTATION_COMPLEX(,)))) f;

int main(int argc, char* argv[])
{
	int i ;

	int __attribute((annotate("target('n') " ANNOTATION_RANGE_N))) n = atoi(argv[1]);
	std::string outputFilename 	= argv[2];

	// prepare the output file for writting the theta values
	std::ofstream outputFileHandler;
	outputFileHandler.open(outputFilename);
	outputFileHandler.precision(5);

	// create the arrays
	x 		= (Complex*)malloc(n * sizeof (Complex));
	f 		= (Complex*)malloc(n * sizeof (Complex));
	indices = (int*)malloc(n * sizeof (int));

	if(x == NULL || f == NULL || indices == NULL)
	{
		std::cout << "cannot allocate memory for the triangle coordinates!" << std::endl;
		return -1 ;
	}

	int K = n;

	for(i = 0;i < K ; i++)
	{
		x[i].real = i < (K / 100) ? 1.0 : 0.0;
		x[i].imag = 0 ;
	}

	AxBenchTimer timer;

	radix2DitCooleyTykeyFft(K, indices, x, f) ;

	uint64_t time = timer.nanosecondsSinceInit();
	std::cout << "kernel time = " << ((double)time) / 1000000000.0 << " s\n";

	for(i = 0;i < K ; i++)
	{
	  outputFileHandler << f[i].real << " " << f[i].imag << " " << indices[i] << std::endl;
	}


	outputFileHandler.close();

	return 0 ;
}

extern "C"//ucciola
void newmain (data* d) {
    x = d-> x;
    f = d-> f;
    int K = *d-> K;
    indices = d-> indices;
    AxBenchTimer timer;

	radix2DitCooleyTykeyFft(K, indices, x, f) ;

	uint64_t time = timer.nanosecondsSinceInit();
	std::cout << "kernel time = " << ((double)time) / 1000000000.0 << " s\n";
}
