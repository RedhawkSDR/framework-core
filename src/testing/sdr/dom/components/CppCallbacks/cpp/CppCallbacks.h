#ifndef CPPCALLBACKS_IMPL_H
#define CPPCALLBACKS_IMPL_H

#include "CppCallbacks_base.h"

class CppCallbacks_i;

class CppCallbacks_i : public CppCallbacks_base
{
    ENABLE_LOGGING
    public:
        CppCallbacks_i(const char *uuid, const char *label);
        ~CppCallbacks_i();
        int serviceFunction();

    private:
        void count_changed(const CORBA::ULong* oldValue, const CORBA::ULong* newValue);
        void constellation_changed(const std::vector<std::complex<float> >* oldValue, const std::vector<std::complex<float> >* newValue);
        void station_changed(const station_struct* oldValue, const station_struct* newValue);
        void servers_changed(const std::vector<endpoint_struct>* oldValue, const std::vector<endpoint_struct>* newValue);
};

#endif
