#include <versioningCompiler/Compiler.hpp>
#include <versioningCompiler/Option.hpp>
#include <versioningCompiler/Utils.hpp>
#include <versioningCompiler/CompilerImpl/SystemCompilerOptimizer.hpp>
#include <iostream>
#include "src/complex.hpp"
#include <fstream>
#include <versioningCompiler/Version.hpp>
#ifndef _PATH_TO_KERNEL                
#define PATH_TO_KERNEL "../src/"    
#else
#define PATH_TO_KERNEL _PATH_TO_KERNEL
#endif
#define fptype float
#define MIN_SVALUE 100000.0
#define MAX_SVALUE -100000.0
fptype minAnnotationRangeN = MIN_SVALUE;
fptype maxAnnotationRangeN = MAX_SVALUE;
fptype minAnnotationComplexRange = MIN_SVALUE;
fptype maxAnnotationComplexRange = MAX_SVALUE;
static int* indices;

typedef struct data_ {
    int* K;
    int* indices;
    Complex* x;
    Complex* f;
} data;

typedef void (*kernel_t)(data*);
vc::version_ptr_t dynamicCompile() {                                             
  const std::string kernel_dir = PATH_TO_KERNEL;
  const std::string kernel_file = kernel_dir + "fft.cpp";
  const std::string functionName = "newmain";                                                            
  const vc::opt_list_t opt_list = {
    vc::make_option("-O0"),
    vc::make_option("-g3"),
    vc::make_option("-I/home/vagrant/TAFFO-new/test/axbench-static/common/src"),
    vc::make_option("-I"+std::string(PATH_TO_KERNEL)),
    vc::make_option("-D_MIN_ANNOTATION_RANGE_N="+std::to_string(minAnnotationRangeN)),
    vc::make_option("-D_MAX_ANNOTATION_RANGE_N="+std::to_string(maxAnnotationRangeN)),
    vc::make_option("-D_MIN_ANNOTATION_COMPLEX_RANGE="+std::to_string(minAnnotationComplexRange)),
    vc::make_option("-D_MAX_ANNOTATION_COMPLEX_RANGE="+std::to_string(maxAnnotationComplexRange)),
  };

  const vc::opt_list_t opt_options_list = {
  //  vc::make_option("-load=/usr/local/lib/TaffoInitializer.so"),
  //  vc::make_option("-taffoinit"),
  //  vc::make_option("-load=/usr/local/lib/TaffoVRA.so"),
  //  vc::make_option("-taffoVRA"),
  //  vc::make_option("-load=/usr/local/lib/TaffoDTA.so"),
  //  vc::make_option("-taffodta"),
  //  vc::make_option("-load=/usr/local/lib/LLVMErrorPropagator.so"),
  //  vc::make_option("-errorprop"),
  //  vc::make_option("-load=/usr/local/lib/LLVMFloatToFixed.so"),
	vc::make_option("-load=/usr/local/lib/Taffo.so"),
    vc::make_option("-flttofix"),
    vc::make_option("-errorprop"),
  };
  vc::vc_utils_init();
  vc::Version::Builder builder;
  vc::compiler_ptr_t clang = vc::make_compiler<vc::SystemCompilerOptimizer>(
                                          "TAFFO",
                                          "taffo",
                                          "opt",
                                          ".",
                                          "./test.log",
                                          "/usr/local/bin",
                                          "/usr/local/bin"
                                        );


  builder._compiler = clang;
  builder._fileName_src.push_back(kernel_file);
  builder._fileName_src.push_back(kernel_dir + "complex.cpp");
  builder._fileName_src.push_back(kernel_dir + "fourier.cpp");
  builder._functionName.push_back(functionName);
  builder._optionList = opt_list;
  builder.optOptions(opt_options_list);
  
  std::cerr << "compiling" << std::endl;
  vc::version_ptr_t version =  builder.build();
  if (!clang->hasOptimizer())
    std::cerr << "No optimizer" << std::endl;

  for (auto i: version->getOptOptionList()){
    std::cerr << i.getPrefix() << i.getTag() << i.getValue() << std::endl;
  }
  std::cerr << version->getID() << std::endl;
  bool ok_compile = true;
  if (clang->hasIRSupport()) {
          //version->prepareIR();
          ok_compile = version->compile();
          std::cerr << "clang has IRSupport" << std::endl;
  }

  if (!ok_compile) {std::cerr << "compilation failed" << std::endl;}

  return version;

}

int main(int argc, char **argv){


  /*
   * INPUT DATA
   */

  //std::vector<int> input = {2048, 8192, 65536};
  int n = atoi(argv[1]);

    static Complex* x;
    static Complex* f;
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

	for(int i = 0;i < K ; i++)
	{
		x[i].real = i < (K / 100) ? 1.0 : 0.0;
        if (x[i].real < minAnnotationComplexRange) minAnnotationComplexRange = x[i].real;
        if (x[i].real > maxAnnotationComplexRange) maxAnnotationComplexRange = x[i].real;
		x[i].imag = 0 ;
	}
    minAnnotationRangeN = 1;
    maxAnnotationRangeN = n;
    data* d = (data*)malloc(sizeof(data));
    d->K = &K;
    d->indices = indices;
    d->f = f;
    d->x = x;

    std::cerr << "Done setting input" << std::endl;

  /*
   * DYNAMIC COMPILATION
   */
  std::cerr << "Compiling..." << std::endl;
  auto version = dynamicCompile();
  kernel_t kernel = (kernel_t) version->getSymbol(0);
  
  if (!kernel) { 
    std::cerr << "Error while processing item " << std::endl;
    exit(0);
  }
  std::cerr << "...Done" << std::endl;

  /*
   * EXECUTION OF KERNEL
   */

  std::cerr << "Execution of Kernel" << std::endl;
  kernel(d);
  std::cerr << "...Done" << std::endl;

  /*
   * WRITE OUTPUT DATA
   */

	std::string outputFilename 	= argv[2];

	// prepare the output file for writting the theta values
	std::ofstream outputFileHandler;
	outputFileHandler.open(outputFilename);
	outputFileHandler.precision(5);

	for(int i = 0;i < K ; i++)
	{
	  outputFileHandler << f[i].real << " " << f[i].imag << " " << indices[i] << std::endl;
	}

    std::cerr << "Done writing output" << std::endl;

	outputFileHandler.close();

	return 0 ;

}
