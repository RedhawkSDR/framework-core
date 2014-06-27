#ifndef TESTLOGGINGAPI_IMPL_BASE_H
#define TESTLOGGINGAPI_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>


class TestLoggingAPI_base : public Resource_impl, protected ThreadedComponent
{
    public:
        TestLoggingAPI_base(const char *uuid, const char *label);
        ~TestLoggingAPI_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // TESTLOGGINGAPI_IMPL_BASE_H
