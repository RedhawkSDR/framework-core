
#ifndef COMPONENT_H
#define	COMPONENT_H
#include "Resource_impl.h"

class Component : public Resource_impl {
public:
    Component(const char* _uuid);
    Component(const char* _uuid, const char *label);
    virtual ~Component();
    void setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior);
    redhawk::ApplicationContainer* getApplication() {
        return this->_app;
    }
private:
    redhawk::ApplicationContainer *_app;

};

#endif	/* COMPONENT_H */

