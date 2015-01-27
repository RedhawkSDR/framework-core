#ifndef CPP_COMP_IMPL_BASE_H
#define CPP_COMP_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class cpp_comp_base : public Component, protected ThreadedComponent
{
    public:
        cpp_comp_base(const char *uuid, const char *label);
        ~cpp_comp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string app_id;
        std::string dom_id;
        short number_components;

    private:
};
#endif // CPP_COMP_IMPL_BASE_H
