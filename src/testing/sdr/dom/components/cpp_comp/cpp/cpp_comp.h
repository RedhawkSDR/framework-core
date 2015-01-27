#ifndef CPP_COMP_IMPL_H
#define CPP_COMP_IMPL_H

#include "cpp_comp_base.h"

class cpp_comp_i : public cpp_comp_base
{
    ENABLE_LOGGING
    public:
        cpp_comp_i(const char *uuid, const char *label);
        ~cpp_comp_i();
        int serviceFunction();
};

#endif // CPP_COMP_IMPL_H
