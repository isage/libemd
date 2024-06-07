#ifndef __STUBWRITER_H_
#define __STUBWRITER_H_

#include "../emd-parse.h"
#include <string>
#include <vector>

class StubWriter {
    public:
        StubWriter(ngpImports* imp, bool weak) : _imp(imp), _weak(weak) {}
        ~StubWriter() {}
        virtual void make_stub() {}
    protected:
        std::vector<std::string> _files;
        ngpImports* _imp;
        bool _weak;
};

#endif