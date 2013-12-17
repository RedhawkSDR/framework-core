#include "CppCallbacks_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

CppCallbacks_base::CppCallbacks_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    serviceThread(0)
{
    construct();
}

void CppCallbacks_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void CppCallbacks_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void CppCallbacks_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<CppCallbacks_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void CppCallbacks_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
    	Resource_impl::stop();
    }
}

void CppCallbacks_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();


    Resource_impl::releaseObject();
}

void CppCallbacks_base::loadProperties()
{
    addProperty(count,
                0,
                "count",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    // Set the sequence with its initial values
    constellation.push_back(std::complex<float> (1.0,0.0));
    constellation.push_back(std::complex<float> (0.0,1.0));
    constellation.push_back(std::complex<float> (-1.0,0.0));
    constellation.push_back(std::complex<float> (0.0,-1.0));
    addProperty(constellation,
                constellation,
                "constellation",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(callbacks_run,
                "callbacks_run",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(station,
                station_struct(),
                "station",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(servers,
                "servers",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}


