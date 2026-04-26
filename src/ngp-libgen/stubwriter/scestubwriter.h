#ifndef __SCESTUBWRITER_H__
#define __SCESTUBWRITER_H__

#include "stubwriter.h"
#include <ario/ario.hpp>

class SceStubWriter : public StubWriter {
    public:
        SceStubWriter(ngpImports *imp, bool weak);
        ~SceStubWriter();
        void make_stub() override;

    private:
        void make_head_stub(ngpImportsLib *library, ARIO::ario& ar);
        void make_nid_stub(ngpImportsLib *library, ARIO::ario& ar);
        void make_func_stubs(ngpImportsLib *library, ARIO::ario& ar);
        void make_var_stubs(ngpImportsLib *library, ARIO::ario& ar);
};

#endif