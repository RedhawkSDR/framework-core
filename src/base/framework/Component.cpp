#include "ossie/Component.h"

Component::Component(const char* _uuid) : Resource_impl (_uuid) {
    this->_app = NULL;
}

Component::Component(const char* _uuid, const char *label) : Resource_impl (_uuid, label) {
    this->_app = NULL;
}

Component::~Component() {
    if (this->_app != NULL)
        delete this->_app;
}

void Component::setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior)
{
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    Resource_impl::setAdditionalParameters(softwareProfile, application_registrar_ior);
    CORBA::Object_var applicationRegistrarObject = orb->string_to_object(application_registrar_ior.c_str());
    CF::ApplicationRegistrar_ptr applicationRegistrar = ossie::corba::_narrowSafe<CF::ApplicationRegistrar>(applicationRegistrarObject);
    if (!CORBA::is_nil(applicationRegistrar)) {
        this->_app = new redhawk::ApplicationContainer(applicationRegistrar->app());
        return;
    }
    this->_app = new redhawk::ApplicationContainer();
}
