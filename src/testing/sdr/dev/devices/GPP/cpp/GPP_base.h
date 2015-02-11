#ifndef GPP_IMPL_BASE_H
#define GPP_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/ExecutableDevice_impl.h>
#include <ossie/ThreadedComponent.h>

#include <ossie/PropertyInterface.h>
#include <ossie/MessageInterface.h>
#include "struct_props.h"

class GPP_base : public ExecutableDevice_impl, protected ThreadedComponent
{
    public:
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~GPP_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        std::string processor_name;
        std::string os_name;
        std::string os_version;
        std::string hostName;
        bool useScreen;
        double loadCapacity;
        CORBA::Long mcastnicIngressCapacity;
        CORBA::Long memCapacity;
        double loadCapacityPerCore;
        float reserved_capacity_per_component;
        short processor_cores;
        CORBA::Long loadThreshold;
        std::vector<std::string> nic_interfaces;
        std::vector<std::string> available_nic_interfaces;
        nic_allocation_struct nic_allocation;
        advanced_struct advanced;
        threshold_event_struct threshold_event;
        thresholds_struct thresholds;
        std::vector<nic_allocation_status_struct_struct> nic_allocation_status;
        std::vector<nic_metrics_struct_struct> nic_metrics;
        std::vector<interfaces_struct> networkMonitor;

        // Ports
        PropertyEventSupplier *propEvent;
        MessageSupplierPort *MessageEvent_out;

    private:
        void construct();
};
#endif // GPP_IMPL_BASE_H
