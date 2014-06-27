#include <iostream>
#include "ossie/ossieSupport.h"

#include "TestLoggingAPI.h"
int main(int argc, char* argv[])
{
    TestLoggingAPI_i* TestLoggingAPI_servant;
    Resource_impl::start_component(TestLoggingAPI_servant, argc, argv);
    return 0;
}

