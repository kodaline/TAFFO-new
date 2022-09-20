#include <versioningCompiler/Compiler.hpp>
#include <versioningCompiler/Option.hpp>
#include <versioningCompiler/Utils.hpp>
#include <versioningCompiler/CompilerImpl/SystemCompilerOptimizer.hpp>
#include <iostream>
#include <fstream>
#include <versioningCompiler/Version.hpp>
#include "src/tritri.cpp"
#ifndef _PATH_TO_KERNEL                
#define PATH_TO_KERNEL "src/"    
#else
#define PATH_TO_KERNEL _PATH_TO_KERNEL
#endif
#define fptype float
typedef struct OptionData_ {
        fptype r;
        fptype divq;
        fptype v;
        fptype t;
        char OptionType;
        fptype divs;
        fptype DGrefval;

} OptionData;

typedef struct blob_ {
    fptype *V0;
    fptype *V1;
    fptype *V2;
    fptype *U0;
    fptype *U1;
    fptype *U2;
    int * output;
} blob;

#define MIN_SVALUE 100000.0
#define MAX_SVALUE -100000.0
fptype minXyz = MIN_SVALUE;
fptype maxXyz = MAX_SVALUE;
fptype minA = MIN_SVALUE;
fptype maxA = MAX_SVALUE;
fptype minAdj = MIN_SVALUE;
fptype maxAdj = MAX_SVALUE;

OptionData *data;
fptype *s;      // spot price  // TEMPORARY: USED ONLY BY PARSER
fptype *stk;    // strike price // TEMPORARY: USED ONLY BY PARSER
fptype *prices;
int numOptions;
typedef int* (*kernel_t)(blob* &b);

vc::version_ptr_t dynamicCompile() {                                             
  const std::string kernel_dir = PATH_TO_KERNEL;
  const std::string kernel_file = kernel_dir + "jmeint.cpp";
  const std::string functionName = "newmain";                                                            
  const vc::opt_list_t opt_list = {
    vc::make_option("-O0"),
    vc::make_option("-g3"),
    vc::make_option("-I/home/vagrant/TAFFO-new/test/axbench-static/common/src"),
    vc::make_option("-D_MIN_XYZ_RANGE="+std::to_string(minXyz)),
    vc::make_option("-D_MAX_XYZ_RANGE="+std::to_string(maxXyz)),
    vc::make_option("-D_MIN_A_RANGE="+std::to_string(minA)),
    vc::make_option("-D_MAX_A_RANGE="+std::to_string(maxA)),
    vc::make_option("-D_MIN_ADJ_RANGE="+std::to_string(minAdj)),
    vc::make_option("-D_MAX_ADJ_RANGE="+std::to_string(maxAdj)),
  };

  const vc::opt_list_t opt_options_list = {
    // vc::make_option("-load=/usr/local/lib/TaffoInitializer.so"),
    // vc::make_option("-taffoinit"),
    // vc::make_option("-load=/usr/local/lib/TaffoVRA.so"),
    // vc::make_option("-taffoVRA"),
    // vc::make_option("-load=/usr/local/lib/TaffoDTA.so"),
    // vc::make_option("-taffodta"),
    // vc::make_option("-load=/usr/local/lib/LLVMErrorPropagator.so"),
    // vc::make_option("-errorprop"),
    // vc::make_option("-load=/usr/local/lib/LLVMFloatToFixed.so"),
	 vc::make_option("-load=/usr/local/lib/Taffo.so"),
     vc::make_option("-flttofix"),
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
   * READ INPUT DATA
   */

  std::string inputFilename = argv[1];
  std::string outputFilename = argv[2];
  int i, x, n;
  // prepare the output file for writting the theta values
  std::ofstream outputFileHandler;
  outputFileHandler.open(outputFilename);
  outputFileHandler.precision(5);
  
  // prepare the input file for reading the theta data
  std::ifstream inputFileHandler (inputFilename, std::ifstream::in);
  
  // first line defins the number of enteries
  inputFileHandler >> n;
  
  // create the directory for storing data
  float* xyz = (float*)malloc(n * 6 * 3 * sizeof(float));
  if(xyz == NULL)
  {
		std::cout << "cannot allocate memory for the triangle coordinates!" << std::endl;
		return -1 ;
	}

  i = 0;
	while (i < n)
	{
		float a[18];
		inputFileHandler >> 	a[0] 	>> 	a[1] 	>> 	a[2] 	>> a[3] 	>> a[4] 		>> a[5] >>
						a[6]	>>	a[7]	>> 	a[8]	>> a[9]		>> a[10]		>> a[11] >>
						a[12]	>> 	a[13] 	>> 	a[14] 	>> a[15] 	>> a[16]	 	>> a[17];
		for(int j = 0 ; j < 18; j++)
		{
            if (a[j] < minA) minA = a[j];
            if (a[j] > maxA) maxA = a[j];

			xyz[i * 18 + j] = a[j];
		}
		i++;
	}
  minXyz = minA;
  maxXyz = maxA;

  int totalCount = 0;
  int outputAggreg[6] = {0};

  /*
   * DYNAMIC COMPILATION
   */
  std::cerr << "Compiling..." << std::endl;
  auto version = dynamicCompile();
  kernel_t f = (kernel_t) version->getSymbol(0);
  
  if (!f) { 
    std::cerr << "Error while processing item " << std::endl;
    exit(0);
  }
  std::cerr << "...Done" << std::endl;
  std::cerr << "Execution of Kernel" << std::endl;
  /*
   * EXECUTION OF KERNEL
   */

 	for(i = 0 ; i < (n * 6 * 3); i += 6 * 3)
	{

		int* output = new int;
    *output = -1;

#pragma parrot(input, "jmeint", [18]dataIn)

  float* V0 = xyz + i + 0 * 3;
  float* V1 = xyz + i + 1 * 3;
  float* V2 = xyz + i + 2 * 3;
  float* U0 = xyz + i + 3 * 3;
  float* U1 = xyz + i + 4 * 3;
  float* U2 = xyz + i + 5 * 3;


  blob* b = (blob *) malloc(sizeof(blob));

  b->V0 = V0;
  b->V1 = V1;
  b->V2 = V2;
  b->U0 = U0;
  b->U1 = U1;
  b->U2 = U2;
  b->output = output;
  x = *f(b);
//		x = tri_tri_intersect(
//				xyz + i + 0 * 3, xyz + i + 1 * 3, xyz + i + 2 * 3,
//				xyz + i + 3 * 3, xyz + i + 4 * 3, xyz + i + 5 * 3,
//				res, &output);

  /*
   * WRITE OUTPUT DATA
   */

  #pragma parrot(output, "jmeint", [2]<0.2; 0.8>dataOut)

    outputAggreg[*output] += 1;
    PRINT_INSTR("exit type = %d\n", output);
		outputFileHandler << x << " 0 0 " << *output << std::endl;
		
	}

	for (int i=0; i<6; i++)
	  PRINT_INSTR("exit type %d total = %d\n", i, outputAggreg[i]);
	
	outputFileHandler.close();
	inputFileHandler.close();

	free(xyz);
	xyz = NULL;

  std::cerr << "...Done" << std::endl;


  return 0;
}
