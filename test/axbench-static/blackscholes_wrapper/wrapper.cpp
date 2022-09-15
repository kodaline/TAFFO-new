#include <versioningCompiler/Compiler.hpp>
#include <versioningCompiler/Option.hpp>
#include <versioningCompiler/Utils.hpp>
#include <versioningCompiler/CompilerImpl/SystemCompilerOptimizer.hpp>
#include <iostream>
#include <versioningCompiler/Version.hpp>
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
fptype minSptprice = MIN_SVALUE;
fptype maxSptprice = MAX_SVALUE;
fptype minStrike = MIN_SVALUE;
fptype maxStrike = MAX_SVALUE;
fptype minVolatility = MIN_SVALUE;
fptype maxVolatility = MAX_SVALUE;
fptype minOtime = MIN_SVALUE;
fptype maxOtime = MAX_SVALUE;
fptype minRate = MIN_SVALUE;
fptype maxRate = MAX_SVALUE;

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
    vc::make_option("-D_MIN_SPTPRICE_RANGE="+std::to_string(minSptprice)),
    vc::make_option("-D_MAX_SPTPRICE_RANGE="+std::to_string(maxSptprice)),
    vc::make_option("-D_MIN_STRIKE_RANGE="+std::to_string(minStrike)),
    vc::make_option("-D_MAX_STRIKE_RANGE="+std::to_string(maxStrike)),
    vc::make_option("-D_MIN_RATE_RANGE="+std::to_string(minRate)),
    vc::make_option("-D_MAX_RATE_RANGE="+std::to_string(maxRate)),
    vc::make_option("-D_MIN_VOLATILITY_RANGE="+std::to_string(minVolatility)),
    vc::make_option("-D_MAX_VOLATILITY_RANGE="+std::to_string(maxVolatility)),
    vc::make_option("-D_MIN_OTIME_RANGE="+std::to_string(minOtime)),
    vc::make_option("-D_MAX_OTIME_RANGE="+std::to_string(maxOtime)),
    vc::make_option("-D_MIN_INPUTXX_RANGE=30"),
    vc::make_option("-D_MAX_INPUTXX_RANGE=7450"),
    vc::make_option("-D_MIN_TIMET_RANGE=0"),
    vc::make_option("-D_MAX_TIMET_RANGE=1"),
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
  std::cerr << "Reading input data..." << std::endl;
  FILE *file;
  int i;
  int loopnum;
  fptype * buffer;
  int * buffer2;
  int rv;
  fflush(NULL);
  char *inputFile = argv[1];
  char *outputFile = argv[2];

  //Read input data from file
  file = fopen(inputFile, "r");
  if(file == NULL) {
    printf("ERROR: Unable to open file `%s'.\n", inputFile);
    exit(1);
  }
  rv = fscanf(file, "%i", &numOptions);
  if(rv != 1) {
    printf("ERROR: Unable to read from file `%s'.\n", inputFile);
    fclose(file);
    exit(1);
  }


  // alloc spaces for the option data
  data = (OptionData*)malloc(numOptions*sizeof(OptionData));
  s = (fptype*)malloc(numOptions*sizeof(fptype));
  stk = (fptype*)malloc(numOptions*sizeof(fptype));
  for ( loopnum = 0; loopnum < numOptions; ++ loopnum )
  {

      rv = fscanf(file, "%f %f ", &s[loopnum], &stk[loopnum]);
      rv += fscanf(file, "%f %f %f %f %c %f %f", &data[loopnum].r, &data[loopnum].divq, &data[loopnum].v, &data[loopnum].t, &data[loopnum].OptionType, &data[loopnum].divs, &data[loopnum].DGrefval);
      if(rv != 9) {
        printf("ERROR: Unable to read from file `%s'.\n", inputFile);
        fclose(file);
        exit(1);
      }
  }
  rv = fclose(file);
  if(rv != 0) {
    printf("ERROR: Unable to close file `%s'.\n", inputFile);
    exit(1);
  }
  std::cerr << "...Done" << std::endl;
  std::cerr << "Analyzing input data..." << std::endl;

#define PAD 256
#define LINESIZE 64

    fptype *sptprice;
    fptype *strike;
    fptype *volatility;
    fptype *otime;
    fptype *rate;
    int * otype;
    #define DIVIDE 120.0
    prices = (fptype*)malloc(numOptions*sizeof(fptype));

    buffer = (fptype *) malloc(5 * numOptions * sizeof(fptype) + PAD);
    sptprice = (fptype *) (((unsigned long long)buffer + PAD) & ~(LINESIZE - 1));
    strike = sptprice + numOptions;
    rate = strike + numOptions;
    volatility = rate + numOptions;
    otime = volatility + numOptions;

    buffer2 = (int *) malloc(numOptions * sizeof(fptype) + PAD);
    otype = (int *) (((unsigned long long)buffer2 + PAD) & ~(LINESIZE - 1));

    for (int i=0; i<numOptions; i++) {
        otype[i]      = (data[i].OptionType == 'P') ? 1 : 0;
        sptprice[i]   = s[i] / DIVIDE;
        if (sptprice[i] < minSptprice) minSptprice = sptprice[i];
        if (sptprice[i] > maxSptprice) maxSptprice = sptprice[i];
        strike[i]     = stk[i] / DIVIDE;
        if (strike[i] < minStrike) minStrike = strike[i];
        if (strike[i] > maxStrike) maxStrike = strike[i];
        rate[i]       = data[i].r;
        if (rate[i] < minRate) minRate = rate[i];
        if (rate[i] > maxRate) maxRate = rate[i];
        volatility[i] = data[i].v;
        if (volatility[i] < minVolatility) minVolatility = volatility[i];
        if (volatility[i] > maxVolatility) maxVolatility = volatility[i];
        otime[i]      = data[i].t;
        if (otime[i] < minOtime) minOtime = otime[i];
        if (otime[i] > maxOtime) maxOtime = otime[i];

    }

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
