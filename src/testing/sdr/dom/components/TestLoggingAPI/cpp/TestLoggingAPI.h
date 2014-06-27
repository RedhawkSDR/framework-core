#ifndef TESTLOGGINGAPI_IMPL_H
#define TESTLOGGINGAPI_IMPL_H

#include "TestLoggingAPI_base.h"

class TestLoggingAPI_i : public TestLoggingAPI_base
{
    ENABLE_LOGGING
    public:
        TestLoggingAPI_i(const char *uuid, const char *label);
        ~TestLoggingAPI_i();
        int serviceFunction();
};

#endif // TESTLOGGINGAPI_IMPL_H
