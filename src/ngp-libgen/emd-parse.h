#ifndef VITA_IMPORT_H
#define VITA_IMPORT_H

#include <stdint.h>
#include <vector>
#include <string>

struct ngpImportsStub
{
  ngpImportsStub(const std::string& name, uint32_t nid) : name(name), NID(nid) {}
  std::string name;
  uint32_t NID;
};

struct ngpImportsLib
{
    ngpImportsLib(const std::string& name) : name(name) {}
    std::string name;
    std::string nidsuffix;
    std::string stubname;
    uint32_t NID = 0;
    bool is_kernel;
    std::vector<ngpImportsStub*> functions;
    std::vector<ngpImportsStub*> variables;
    uint32_t flags = 0;
};

struct ngpImportsModule
{
    ngpImportsModule(std::string name, uint32_t nid, uint32_t major, uint32_t minor) : name(name), NID(nid), major(major), minor(minor) {}
    std::string name;
    uint32_t NID = 0;
    std::vector<ngpImportsLib*> libs;
    uint32_t major = 0;
    uint32_t minor = 0;
};

struct ngpImports
{
    std::vector<ngpImportsModule*> modules;
};

ngpImports* vita_imports_load(const char *filename, int verbose);

#endif
