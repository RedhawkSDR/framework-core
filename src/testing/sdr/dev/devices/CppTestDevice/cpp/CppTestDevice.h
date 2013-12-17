#ifndef CPPTESTDEVICE_IMPL_H
#define CPPTESTDEVICE_IMPL_H

#include "CppTestDevice_base.h"

class CppTestDevice_i;

class CppTestDevice_i : public CppTestDevice_base
{
    ENABLE_LOGGING
    public:
		static const float MAX_LOAD = 4.0;

		CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~CppTestDevice_i();
        int serviceFunction();

        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

    private:
        bool allocate_memory(const memory_allocation_struct& capacity);
        void deallocate_memory(const memory_allocation_struct& capacity);

        bool allocate_load(const float& capacity);
        void deallocate_load(const float& capacity);
};

#endif
