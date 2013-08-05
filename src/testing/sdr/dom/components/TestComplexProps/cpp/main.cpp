#include <iostream>
#include "ossie/ossieSupport.h"

#include "TestComplexProps.h"

int main(int argc, char* argv[])
{
    TestComplexProps_i* TestComplexProps_servant;
    Resource_impl::start_component(TestComplexProps_servant, argc, argv);
}
