#ifndef __VITASDKSTUBWRITER_H__
#define __VITASDKSTUBWRITER_H__

#include "stubwriter.h"

class VitasdkStubWriter : public StubWriter {
    public:
        VitasdkStubWriter(ngpImports *imp, bool weak);
        ~VitasdkStubWriter();
        void make_stub() override;

    private:
};

#endif