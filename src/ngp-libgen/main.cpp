#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <fstream>
#include <string>
#include <fmt/core.h>
#include "emd-parse.h"
#include "templates.h"

int generate_assembly(std::vector<ngpImports*>& imports)
{
  FILE *fp;

  for (auto& imp : imports)
  {
    for (auto& module : imp->modules)
    {
      for (auto& library : module->libs)
      {
        for (auto& function: library->functions)
        {

          std::string filename = fmt::format("{}_{}_{}.S", module->name, library->name, function->name);

          if ((fp = fopen(filename.c_str(), "w")) == NULL)
            return 0;

          fmt::print(fp, ASM_FUNC_TEMPLATE,
              fmt::arg("libname", library->name),
              fmt::arg("funcname", function->name),
              fmt::arg("attr", ((library->flags >> 16) & 0xFFFF)),
              fmt::arg("libnid", library->NID),
              fmt::arg("funcnid", function->NID)
          );

          fclose(fp);
        }

        for (auto& variable: library->variables)
        {
          std::string filename = fmt::format("{}_{}_{}.S", module->name, library->name, variable->name);

          FILE *fp;
          if ((fp = fopen(filename.c_str(), "w")) == NULL)
            return 0;

          fmt::print(fp, ASM_VAR_TEMPLATE,
              fmt::arg("libname", library->name),
              fmt::arg("varname", variable->name),
              fmt::arg("attr", ((library->flags >> 16) & 0xFFFF)),
              fmt::arg("libnid", library->NID),
              fmt::arg("varnid", variable->NID)
          );

          fclose(fp);
        }
      }
    }
  }

  return 1;
}

int generate_cmake_src(FILE *fp, ngpImportsModule *module, ngpImportsLib *library)
{
  if (!library->functions.size() && !library->variables.size())
    return 0;

  fmt::print(fp, "set({}_ASM\n", library->name);

  for (auto& function : library->functions)
  {
    fmt::print(fp, "  \"{}_{}_{}.S\"\n", module->name, library->name, function->name);
  }

  for (auto& variable : library->variables)
  {
    fmt::print(fp, "  \"{}_{}_{}.S\"\n", module->name, library->name, variable->name);
  }

  fmt::print(fp, ")\n\n");

  fmt::print(fp, CMAKE_LIB_TEMPLATE, fmt::arg("library", library->name), fmt::arg("firmware", fmt::format("{}{}", module->major, module->minor)));

  return 1;
}

int generate_cmake(std::vector<ngpImports*>& imports)
{
  FILE *fp;

  if ((fp = fopen("CMakeLists.txt", "w")) == NULL) {
    return 0;
  }

  fmt::print(fp, CMAKE_MAIN_TEMPLATE);

  for (auto& imp : imports)
  {
    for (auto& module : imp->modules)
    {
      fmt::print(fp, CMAKE_PROJECT_TEMPLATE, fmt::arg("project", module->name));

      for (auto& lib : module->libs)
      {
        generate_cmake_src(fp, module, lib);
      }
    }
  }

  fclose(fp);
  return 1;
}

int main(int argc, const char **argv)
{
  if (argc < 3) {
      fmt::print(stderr,
        "vita-libs-gen-emd\n"
        "usage: vita-libs-gen-emd library.emd [extra.emd ...] output-dir\n"
      );
    return -1;
  }

  int imports_count = argc - 2;

  std::vector<ngpImports *> imports;

  int i;
  for (i = 0; i < imports_count; i++)
  {
    ngpImports *imp = vita_imports_load(argv[i + 1], 1);

    if (imp == NULL) {
        return -1;
    }

    imports.push_back(imp);
  }

  mkdir(argv[argc - 1], 0777);

  if (chdir(argv[argc - 1])) {
    perror(argv[argc - 1]);
    return -1;
  }

  if (!generate_assembly(imports)) {
    fmt::print(stderr, "Error generating the assembly file\n");
    return -1;
  }

  if (!generate_cmake(imports)) {
    fmt::print(stderr, "Error generating the CMakeLists.txt\n");
    return -1;
  }

  for (i = 0; i < imports.size(); i++)
  {
    delete imports[i];
  }

  return 0;
}
