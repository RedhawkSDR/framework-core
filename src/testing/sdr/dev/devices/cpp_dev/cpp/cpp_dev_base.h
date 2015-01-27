#ifndef CPP_DEV_IMPL_BASE_H
#define CPP_DEV_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>


class cpp_dev_base : public Device_impl, protected ThreadedComponent
{
    public:
        cpp_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        cpp_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        cpp_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        cpp_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~cpp_dev_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        std::string devmgr_id;
        std::string dom_id;

    private:
        void construct();
};
#endif // CPP_DEV_IMPL_BASE_H
