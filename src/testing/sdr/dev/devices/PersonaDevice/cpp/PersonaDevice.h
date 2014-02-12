#ifndef PERSONADEVICE_IMPL_H
#define PERSONADEVICE_IMPL_H

#include "PersonaDevice_persona_base.h"

class PersonaDevice_i;

class PersonaDevice_i : public PersonaDevice_persona_base
{
    ENABLE_LOGGING
    public:
        PersonaDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        PersonaDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        PersonaDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        PersonaDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~PersonaDevice_i();
        int serviceFunction();
        CORBA::Boolean allocateCapacity(const CF::Properties& capacities) 
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException);
        void deallocateCapacity(const CF::Properties& capacities) 
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);

    protected:
        void hwLoadRequest(CF::Properties& request);
};

#endif
