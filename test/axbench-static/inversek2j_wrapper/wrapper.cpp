#include <versioningCompiler/Compiler.hpp>
#include <versioningCompiler/Option.hpp>
#include <versioningCompiler/Utils.hpp>
#include <versioningCompiler/CompilerImpl/SystemCompilerOptimizer.hpp>
#include <iostream>
#include <fstream> 
#include <cmath>
#include <versioningCompiler/Version.hpp>
#ifndef _PATH_TO_KERNEL                
#define PATH_TO_KERNEL "./src/"    
#else
#define PATH_TO_KERNEL _PATH_TO_KERNEL
#endif
#define fptype float
#define MIN_SVALUE 100000.0
#define MAX_SVALUE -100000.0
fptype minTheta1 = MIN_SVALUE;
fptype maxTheta1 = MAX_SVALUE;
fptype minTheta2 = MIN_SVALUE;
fptype maxTheta2 = MAX_SVALUE;
fptype minSqtmp = MIN_SVALUE;
fptype maxSqtmp = MAX_SVALUE;

typedef void (*kernel_t)(float*, int);

  float theta1; 
  float theta2; 
  float l1;
  float l2;
  float* t1t2xy;

vc::version_ptr_t dynamicCompile() {                                             
  const std::string kernel_dir = PATH_TO_KERNEL;
  const std::string kernel_file = kernel_dir + "inversek2j.cpp";
  const std::string functionName = "newmain";                                                            
  const vc::opt_list_t opt_list = {
    vc::make_option("-O0"),
    vc::make_option("-g3"),
    vc::make_option("-I/home/vagrant/TAFFO-new/test/axbench-static/common/src"),
    vc::make_option("-I" + kernel_dir),
    vc::make_option("-D_MIN_THETA1_RANGE="+std::to_string(minTheta1)),
    vc::make_option("-D_MAX_THETA1_RANGE="+std::to_string(maxTheta1)),
    vc::make_option("-D_MIN_THETA2_RANGE="+std::to_string(minTheta2)),
    vc::make_option("-D_MAX_THETA2_RANGE="+std::to_string(maxTheta2)),
    vc::make_option("-D_MIN_SQTMP_RANGE="+std::to_string(minSqtmp)),
    vc::make_option("-D_MAX_SQTMP_RANGE="+std::to_string(maxSqtmp)),
    vc::make_option("-D_L1_VAL="+std::to_string(l1)),
    vc::make_option("-D_L2_VAL="+std::to_string(l2)),

  };

  const vc::opt_list_t opt_options_list = {
    //vc::make_option("-load=/usr/local/lib/TaffoInitializer.so"),
    //vc::make_option("-taffoinit"),
    //vc::make_option("-load=/usr/local/lib/TaffoVRA.so"),
    //vc::make_option("-taffoVRA"),
    //vc::make_option("-load=/usr/local/lib/TaffoDTA.so"),
    //vc::make_option("-taffodta"),
    //vc::make_option("-load=/usr/local/lib/LLVMErrorPropagator.so"),
    vc::make_option("-load=/usr/local/lib/Taffo.so"),
    vc::make_option("-errorprop"),
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
  builder._fileName_src.push_back(kernel_dir + "kinematics.cpp");
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
  int n;
  std::string inputFilename	= argv[1];
  std::string outputFilename = argv[2];
  
  //prepare the output file for writting the theta values
  std::ofstream outputFileHandler;
  outputFileHandler.open(outputFilename);
  
  //prepare the input file for reading the theta data
  std::ifstream inputFileHandler (inputFilename, std::ifstream::in);
  
  // first line define the number of enteries
  inputFileHandler >> n;
  l1 = atof(argv[3]);
  l2 = atof(argv[4]);
  t1t2xy = (float*)malloc(n * 2 * 2 * sizeof(float));


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

  if(t1t2xy == NULL)
  {
    std::cerr << "# Cannot allocate memory for the coordinates an angles!" << std::endl;
		return -1;
  }
  for (int i=0; i<n*2*2; i+=2+2) {
      inputFileHandler >> theta1 >> theta2;
      t1t2xy[i] = theta1;
      t1t2xy[i+1] = theta2;
      float x = l1 * cos(theta1) + l2 * cos(theta1 + theta2);
      float y = l1 * sin(theta1) + l2 * sin(theta1 + theta2);
      float sqtmp = (x*x + y*y);
      if (sqtmp < minSqtmp) minSqtmp = sqtmp;
      if (sqtmp > maxSqtmp) maxSqtmp = sqtmp;
      if (theta1 < minTheta1) minTheta1 = theta1;
      if (theta1 > maxTheta1) maxTheta1 = theta1;
      if (theta2 < minTheta2) minTheta2 = theta2;
      if (theta2 > maxTheta2) maxTheta2 = theta2;

  }
  f(t1t2xy, n);

  std::cerr << "...Done" << std::endl;

  /*
   * WRITE OUTPUT DATA
   */

  std::cerr << "Write Output" << std::endl;
  for(int i = 0 ; i < n * 2 * 2 ; i += 2 * 2)
	{
		outputFileHandler <<  t1t2xy[i+0] << "\t" << t1t2xy[i+1] << "\n";
	}

  std::cerr << "...Done" << std::endl;

  std::cerr << "Free mem" << std::endl;

	inputFileHandler.close();
	outputFileHandler.close();

	free(t1t2xy) ; 
	t1t2xy = NULL ;

  std::cerr << "...Done" << std::endl;


  return 0;
}
