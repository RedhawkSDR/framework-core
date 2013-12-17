#include <iostream>
#include "ossie/ossieSupport.h"

#include "CppCallbacks.h"
int main(int argc, char* argv[])
{
    CppCallbacks_i* CppCallbacks_servant;
    Resource_impl::start_component(CppCallbacks_servant, argc, argv);
    return 0;
}

