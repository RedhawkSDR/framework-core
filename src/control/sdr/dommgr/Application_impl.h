/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */


#ifndef APPLICATION_H
#define APPLICATION_H

#include <vector>
#include <string>
#include <map>
#include <set>

#include <boost/thread/condition_variable.hpp>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>
#include <ossie/Logging_impl.h>

#include "applicationSupport.h"
#include "connectionSupport.h"


class DomainManager_impl;
class ApplicationRegistrar_impl;

class Application_impl : public virtual POA_CF::Application, public Logging_impl
{
    ENABLE_LOGGING
    friend class DomainManager_impl;

protected:
    CF::Resource_var assemblyController;

public:

    Application_impl (const std::string& id, const std::string& name, const std::string& profile,
                      DomainManager_impl* domainManager, const std::string& waveformContextName,
                      CosNaming::NamingContext_ptr waveformContext, bool trusted);
    
    void populateApplication (CF::Resource_ptr _assemblyController,
                              std::vector<ossie::DeviceAssignmentInfo>& _devSequence,
                              std::vector<CF::Resource_var> _startSeq,
                              std::vector<ossie::ConnectionNode>& connections,
                              std::vector<std::string> allocationIDs);

    ~Application_impl ();

    static PortableServer::ObjectId* Activate(Application_impl* application);

    char* identifier () throw (CORBA::SystemException);
    CORBA::Boolean started ()
        throw (CORBA::SystemException);
    void start ()
        throw (CF::Resource::StartError, CORBA::SystemException);
    void stop ()
        throw (CF::Resource::StopError, CORBA::SystemException);

    /// The core framework provides an implementation for this method.
    void configure (const CF::Properties& configProperties)
        throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

    /// The core framework provides an implementation for this method.
    void query (CF::Properties& configProperties)
        throw (CF::UnknownProperties, CORBA::SystemException);

    void initialize ()
        throw (CF::LifeCycle::InitializeError, CORBA::SystemException);
        
    void releaseObject ()
        throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);
        
    CORBA::Object_ptr getPort (const char*)
        throw (CORBA::SystemException, CF::PortSupplier::UnknownPort);
        
    void runTest (CORBA::ULong, CF::Properties&)
        throw (CORBA::SystemException, CF::UnknownProperties, CF::TestableObject::UnknownTest);
    
    char* profile () throw (CORBA::SystemException);
    
    char* softwareProfile () throw (CORBA::SystemException);
    
    char* name () throw (CORBA::SystemException);
    
    bool trusted () throw (CORBA::SystemException);
    
    CF::DeviceAssignmentSequence * componentDevices ()
        throw (CORBA::SystemException);
        
    CF::Application::ComponentElementSequence * componentImplementations ()
        throw (CORBA::SystemException);
        
    CF::Application::ComponentElementSequence * componentNamingContexts ()
        throw (CORBA::SystemException);
        
    CF::Application::ComponentProcessIdSequence * componentProcessIds ()
        throw (CORBA::SystemException);
    
    CF::Components * registeredComponents ();
    
    CF::ApplicationRegistrar_ptr appReg (void);

    void addExternalPort (const std::string&, CORBA::Object_ptr);
    void addExternalProperty (const std::string&, const std::string&, CF::Resource_ptr);

    // Returns true if any connections in this application depend on the given object, false otherwise
    bool checkConnectionDependency (ossie::Endpoint::DependencyType type, const std::string& identifier) const;
    
    void _cleanupActivations();

    // Set component state
    void addComponent(const std::string& identifier, const std::string& profile);
    void setComponentPid(const std::string& identifier, unsigned long pid);
    void setComponentNamingContext(const std::string& identifier, const std::string& name);
    void setComponentImplementation(const std::string& identifier, const std::string& implementationId);
    void setComponentDevice(const std::string& identifier, CF::Device_ptr device);
    void addComponentLoadedFile(const std::string& identifier, const std::string& fileName);

    void releaseComponents();
    void terminateComponents();
    void unloadComponents();

    bool waitForComponents(std::set<std::string>& identifiers, int timeout);
    
private:
    Application_impl (); // No default constructor
    Application_impl(Application_impl&);  // No copying

    void registerComponent(CF::Resource_ptr resource);

    bool _checkRegistrations(std::set<std::string>& identifiers);

    const std::string _identifier;
    const std::string _sadProfile;
    const std::string _appName;
    std::vector<ossie::DeviceAssignmentInfo> _componentDevices;
    std::vector<ossie::ConnectionNode> _connections;
    std::vector<CF::Resource_var> _appStartSeq;
    std::vector<std::string> _allocationIDs;
    DomainManager_impl* _domainManager;
    const std::string _waveformContextName;
    CosNaming::NamingContext_var _waveformContext;
    const bool _isTrusted;
    
    ApplicationRegistrar_impl* _registrar;

    ossie::ComponentList _components;

    boost::mutex _registrationMutex;
    boost::condition_variable _registrationCondition;

    std::map<std::string, CORBA::Object_var> _ports;
    std::map<std::string, std::pair<std::string, CF::Resource_ptr> > _properties;

    bool _releaseAlreadyCalled;
    boost::mutex releaseObjectLock;

    ossie::ApplicationComponent* findComponent(const std::string& identifier);

    // Returns externalpropid if one exists based off of compId and
    // internal propId, returns empty string if no external prop exists
    std::string getExternalPropertyId(std::string compId, std::string propId);

    friend class ApplicationRegistrar_impl;
};

class FakeApplication : public virtual POA_CF::Application, public Logging_impl
{

public:

    FakeApplication (const std::string& id, const std::string& name, Application_impl *app);

    ~FakeApplication ();

    static PortableServer::ObjectId* Activate(FakeApplication* fakeApplication);

    char* identifier () throw (CORBA::SystemException) {
        return CORBA::string_dup(_identifier.c_str());
    }
    CORBA::Boolean started ()
        throw (CORBA::SystemException) {
        return this->_app->started();
    };
    void start ()
        throw (CF::Resource::StartError, CORBA::SystemException) {
        throw(CF::Resource::StartError());
    };
    void stop ()
        throw (CF::Resource::StopError, CORBA::SystemException) {
        throw(CF::Resource::StopError());
    };

    /// The core framework provides an implementation for this method.
    void configure (const CF::Properties& configProperties)
        throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException) {
        throw(CF::UnknownProperties());
    };

    /// The core framework provides an implementation for this method.
    void query (CF::Properties& configProperties)
        throw (CF::UnknownProperties, CORBA::SystemException) {
        throw(CF::UnknownProperties());
    };

    void initialize ()
        throw (CF::LifeCycle::InitializeError, CORBA::SystemException) {};
        
    void releaseObject ()
        throw (CF::LifeCycle::ReleaseError, CORBA::SystemException) {};
        
    CORBA::Object_ptr getPort (const char*)
        throw (CORBA::SystemException, CF::PortSupplier::UnknownPort) {
        throw(CF::PortSupplier::UnknownPort());
    };
        
    void runTest (CORBA::ULong, CF::Properties&)
        throw (CORBA::SystemException, CF::UnknownProperties, CF::TestableObject::UnknownTest) {
        throw(CF::TestableObject::UnknownTest());
    };
    
    char* profile () throw (CORBA::SystemException) {
        std::string blank;
        return CORBA::string_dup(blank.c_str());
    };
    
    char* softwareProfile () throw (CORBA::SystemException) {
        std::string blank;
        return CORBA::string_dup(blank.c_str());
    };
    
    char* name () throw (CORBA::SystemException) {
        return CORBA::string_dup(_appName.c_str());
    };
    
    bool trusted () throw (CORBA::SystemException) {
        return false;
    };
    
    CF::DeviceAssignmentSequence * componentDevices ()
        throw (CORBA::SystemException) {
        CF::DeviceAssignmentSequence_var result = new CF::DeviceAssignmentSequence();
        return result._retn();
    };
        
    CF::Application::ComponentElementSequence * componentImplementations ()
        throw (CORBA::SystemException) {
        CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence();
        return result._retn();
    };
        
    CF::Application::ComponentElementSequence * componentNamingContexts ()
        throw (CORBA::SystemException) {
        CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence();
        return result._retn();
    };
        
    CF::Application::ComponentProcessIdSequence * componentProcessIds ()
        throw (CORBA::SystemException) {
        CF::Application::ComponentProcessIdSequence_var result = new CF::Application::ComponentProcessIdSequence();
        return result._retn();
    };
    
    CF::Components * registeredComponents () {
        CF::Components_var result = new CF::Components();
        return result._retn();
    };
    
    CF::ApplicationRegistrar_ptr appReg (void) {
        return CF::ApplicationRegistrar::_nil();
    };
    
protected:
    std::string _identifier;
    std::string _appName;
    Application_impl* _app;
};

#endif
