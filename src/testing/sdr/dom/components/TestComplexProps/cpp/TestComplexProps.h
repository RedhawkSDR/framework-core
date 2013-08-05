#ifndef TESTCOMPLEXPROPS_IMPL_H
#define TESTCOMPLEXPROPS_IMPL_H

#include "TestComplexProps_base.h"

class TestComplexProps_i;

class TestComplexProps_i : public TestComplexProps_base
{
    ENABLE_LOGGING
    public:
        TestComplexProps_i(const char *uuid, const char *label);
        ~TestComplexProps_i();
        int serviceFunction();
};

#endif
