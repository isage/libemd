#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <string>
#include <fmt/core.h>
#include "libs/cxxopts.h"
#include "emd-parse.h"
//#include "templates.h"
#include "stubwriter/scestubwriter.h"
#include "stubwriter/vitasdkstubwriter.h"


int main(int argc, const char **argv)
{

  cxxopts::Options options("ngp-libgen", "PSVita library stub generator");
  options.add_options()
      ("vitasdk", "Generate vitasdk-compatible stub", cxxopts::value<bool>()->default_value("false"))
      ("w,weak", "Generate weak stub", cxxopts::value<bool>()->default_value("false"))
      ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
      ("h,help", "Print usage")
      ("emdfile", "Library emd file to parse", cxxopts::value<std::string>())
  ;

  options.parse_positional({"emdfile"});
  options.positional_help("<emdfile>");
//  options.show_positional_help();
  auto result = options.parse(argc, argv);

  if (result.count("help"))
  {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  if (!result.count("emdfile"))
  {
    std::cout << options.help() << std::endl;
    exit(-1);
  }

  ngpImports *imp = vita_imports_load(result["emdfile"].as<std::string>(), result["verbose"].as<bool>());
  if (!imp)
  {
    exit(-1);
  }

  StubWriter* writer;

  if (result["vitasdk"].as<bool>())
  {
    writer = new VitasdkStubWriter(imp, result["weak"].as<bool>());
  }
  else
  {
    writer = new SceStubWriter(imp, result["weak"].as<bool>());
  }

  writer->make_stub();

  delete writer;
  delete imp;
  return 0;
}
