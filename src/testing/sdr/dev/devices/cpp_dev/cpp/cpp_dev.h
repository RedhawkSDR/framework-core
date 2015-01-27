#ifndef CPP_DEV_IMPL_H
#define CPP_DEV_IMPL_H

#include "cpp_dev_base.h"

class cpp_dev_i : public cpp_dev_base
{
    ENABLE_LOGGING
    public:
        cpp_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        cpp_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        cpp_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        cpp_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~cpp_dev_i();
        int serviceFunction();
        void initialize () throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

    protected:
        void updateUsageState();
};

#endif // CPP_DEV_IMPL_H
