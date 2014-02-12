#ifndef PROGRAMMABLEDEVICE_IMPL_H
#define PROGRAMMABLEDEVICE_IMPL_H

#include "ProgrammableDevice_prog_base.h"

typedef ProgrammableDevice_prog_base<hw_load_request_struct, hw_load_status_struct> ProgrammableDevice_prog_base_type;


class ProgrammableDevice_i;

class ProgrammableDevice_i : public ProgrammableDevice_prog_base_type
{
    ENABLE_LOGGING
    public:
        ProgrammableDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        ProgrammableDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        ProgrammableDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        ProgrammableDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~ProgrammableDevice_i();
        int serviceFunction();
        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

    protected:
        Device_impl* generatePersona(int argc, char* argv[], ConstructorPtr fnptr, const char* libName);
        bool loadHardware(HwLoadStatusStruct& requestStatus);
        void unloadHardware(const HwLoadStatusStruct& requestStatus);
};

#endif
