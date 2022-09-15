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
    fptype *sptprice;
    fptype *strike;
    fptype *volatility;
    fptype *otime;
    fptype *rate;
    int * otype;
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
typedef fptype* (*kernel_t)(blob* &b);

vc::version_ptr_t dynamicCompile() {                                             
  const std::string kernel_dir = PATH_TO_KERNEL;
  const std::string kernel_file = kernel_dir + "blackscholes.cpp";
  const std::string functionName = "newmain";                                                            
  const vc::opt_list_t opt_list = {
    vc::make_option("-O0"),
    vc::make_option("-g3"),
    vc::make_option("-I/home/vagrant/TAFFO/test/axbench-static/common/src"),
    vc::make_option("-D_MIN_XYZ_RANGE="+std::to_string(minXyz)),
    vc::make_option("-D_MAX_XYZ_RANGE="+std::to_string(maxXyz)),
    vc::make_option("-D_MIN_A_RANGE="+std::to_string(minA)),
    vc::make_option("-D_MAX_A_RANGE="+std::to_string(maxA)),
    vc::make_option("-D_MIN_ADJ_RANGE="+std::to_string(minAdj)),
    vc::make_option("-D_MAX_ADJ_RANGE="+std::to_string(maxAdj)),
  };

  const vc::opt_list_t opt_options_list = {
    vc::make_option("-load=/usr/local/lib/TaffoInitializer.so"),
    vc::make_option("-taffoinit"),
    vc::make_option("-load=/usr/local/lib/TaffoVRA.so"),
    vc::make_option("-taffoVRA"),
    vc::make_option("-load=/usr/local/lib/TaffoDTA.so"),
    vc::make_option("-taffodta"),
    vc::make_option("-load=/usr/local/lib/LLVMErrorPropagator.so"),
    vc::make_option("-errorprop"),
    vc::make_option("-load=/usr/local/lib/LLVMFloatToFixed.so"),
    vc::make_option("-flttofix"),
  };
  vc::vc_utils_init();
  vc::Version::Builder builder;
  vc::compiler_ptr_t clang = vc::make_compiler<vc::SystemCompilerOptimizer>(
                                          "llvm-project/clang",
                                          "clang",
                                          "opt",
                                          ".",
                                          "./test.log",
                                          "/usr/local/bin",
                                          "/usr/local/llvm-8/bin"
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

 	for(i = 0 ; i < (n * 6 * 3); i += 6 * 3)
	{

		float __attribute((annotate("target('res') scalar()"))) res[2];
		int output = -1;

#pragma parrot(input, "jmeint", [18]dataIn)

		float* V0 = xyz + i + 0 * 3;
        float* V1 = xyz + i + 1 * 3;
        float* V2 = xyz + i + 2 * 3;
		float* U0 = xyz + i + 3 * 3;
        float* U1 = xyz + i + 4 * 3;
        float* U2 = xyz + i + 5 * 3;
        
        float E1[3],E2[3];
        float N1[3],N2[3],d1,d2;
        SUB(E1,V1,V0);
        SUB(E2,V2,V0);
        CROSS(N1,E1,E2);
        d1=-DOT(N1,V0);
   

  std::cerr << "...Done" << std::endl;

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

  /*
   * EXECUTION OF KERNEL
   */

  std::cerr << "Execution of Kernel" << std::endl;
  blob* b = (blob *) malloc(sizeof(blob));

  b->otype = otype;
  b->sptprice = sptprice;
  b->strike = strike;
  b->rate = rate;
  b->volatility = volatility;
  b->otime = otime;

  prices = f(b);
  std::cerr << "...Done" << std::endl;

  /*
   * WRITE OUTPUT DATA
   */

  std::cerr << "Write Output" << std::endl;
    file = fopen(outputFile, "w");
    if(file == NULL) {
      printf("ERROR: Unable to open file `%s'.\n", outputFile);
      exit(1);
    }
    //rv = fprintf(file, "%i\n", numOptions);
    if(rv < 0) {
      printf("ERROR: Unable to write to file `%s'.\n", outputFile);
      fclose(file);
      exit(1);
    }
    for(i=0; i<numOptions; i++) {
      rv = fprintf(file, "%.18f\n", prices[i]);
      if(rv < 0) {
        printf("ERROR: Unable to write to file `%s'.\n", outputFile);
        fclose(file);
        exit(1);
      }
    }
    rv = fclose(file);
    if(rv != 0) {
      printf("ERROR: Unable to close file `%s'.\n", outputFile);
      exit(1);
    }

  std::cerr << "...Done" << std::endl;

  std::cerr << "Free mem" << std::endl;
    free(data);
    free(prices);
  std::cerr << "...Done" << std::endl;


  return 0;
}
