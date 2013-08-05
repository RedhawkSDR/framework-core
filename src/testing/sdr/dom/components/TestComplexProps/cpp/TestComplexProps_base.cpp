#include "TestComplexProps_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

TestComplexProps_base::TestComplexProps_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    serviceThread(0)
{
    construct();
}

void TestComplexProps_base::construct()
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
void TestComplexProps_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void TestComplexProps_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<TestComplexProps_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void TestComplexProps_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
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

void TestComplexProps_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

void TestComplexProps_base::loadProperties()
{
    addProperty(complexBooleanProp,
                std::complex<bool> (0,1),
                "complexBooleanProp",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexULongProp,
                std::complex<CORBA::ULong> (4,5),
                "complexULongProp",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexShortProp,
                std::complex<short> (4,5),
                "complexShortProp",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexFloatProp,
                std::complex<float> (4.0,5.0),
                "complexFloatProp",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexOctetProp,
                std::complex<unsigned char> (4,5),
                "complexOctetProp",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexCharProp,
                std::complex<char> (4,5),
                "complexCharProp",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexUShort,
                std::complex<unsigned short> (4,5),
                "complexUShort",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexDouble,
                std::complex<double> (4.0,5.0),
                "complexDouble",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexLong,
                std::complex<CORBA::Long> (4,5),
                "complexLong",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexLongLong,
                std::complex<CORBA::LongLong> (4,5),
                "complexLongLong",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexULongLong,
                std::complex<CORBA::ULongLong> (4,5),
                "complexULongLong",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    // Set the sequence with its initial values
    complexFloatSequence.push_back(std::complex<float> (4.0,5.0));
    complexFloatSequence.push_back(std::complex<float> (4.0,5.0));
    complexFloatSequence.push_back(std::complex<float> (4.0,5.0));
    addProperty(complexFloatSequence,
                complexFloatSequence,
                "complexFloatSequence",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(FloatStruct,
                FloatStruct_struct(),
                "FloatStruct",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexFloatStruct,
                complexFloatStruct_struct(),
                "complexFloatStruct",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(FloatStructSequence,
                "FloatStructSequence",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(complexFloatStructSequence,
                "complexFloatStructSequence",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}
