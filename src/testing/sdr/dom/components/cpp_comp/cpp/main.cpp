#include <iostream>
#include "ossie/ossieSupport.h"

#include "cpp_comp.h"
int main(int argc, char* argv[])
{
    cpp_comp_i* cpp_comp_servant;
    Component::start_component(cpp_comp_servant, argc, argv);
    return 0;
}

