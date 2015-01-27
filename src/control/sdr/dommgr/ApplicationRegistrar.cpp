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

#include "ApplicationRegistrar.h"
#include "Application_impl.h"
#include "DomainManager_impl.h"

ApplicationRegistrar_impl::ApplicationRegistrar_impl(CosNaming::NamingContext_ptr context, Application_impl *app) :
    _context(CosNaming::NamingContext::_duplicate(context)),
    _application(app)
{
    this->_fakeApplication = new FakeApplication(app->_identifier, app->_appName, app);
}

ApplicationRegistrar_impl::~ApplicationRegistrar_impl() {
    PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
    PortableServer::POA_var app_poa = dm_poa->find_POA("Applications", 0);

    // Deactivate the application registry, release our reference and reset the
    // local pointer
    PortableServer::ObjectId_var oid = app_poa->servant_to_id(this->_fakeApplication);
    app_poa->deactivate_object(oid);
    delete this->_fakeApplication;
}

CF::Application_ptr ApplicationRegistrar_impl::app() {
    if (!_application->trusted()) {
        return _fakeApplication->_this();
    }
    return _application->_this();
}

CF::DomainManager_ptr ApplicationRegistrar_impl::domMgr() {
    if (!_application->trusted()) {
        return CF::DomainManager::_nil();
    }
    return _application->_domainManager->_this();
}

void ApplicationRegistrar_impl::registerComponent(const char * Name, CF::Resource_ptr obj) throw (CF::InvalidObjectReference, CF::DuplicateName, CORBA::SystemException) {
    ossie::corba::bindObjectToContext(obj, _context, Name);
    _application->registerComponent(obj);
}

// CosNaming::NamingContext interface (supported)
void ApplicationRegistrar_impl::bind(const CosNaming::Name &Name, CORBA::Object_ptr obj) throw (CosNaming::NamingContext::NotFound, 
        CosNaming::NamingContext::CannotProceed, CosNaming::NamingContext::InvalidName, CosNaming::NamingContext::AlreadyBound, CORBA::SystemException) {
    this->_context->bind(Name, obj);
    CF::Resource_var resource = ossie::corba::_narrowSafe<CF::Resource>(obj);
    if (!CORBA::is_nil(resource)) {
        _application->registerComponent(resource);
    }
}
void ApplicationRegistrar_impl::unbind(const CosNaming::Name &Name) throw (CosNaming::NamingContext::NotFound, 
        CosNaming::NamingContext::CannotProceed, CosNaming::NamingContext::InvalidName, CORBA::SystemException) {
    this->_context->unbind(Name);
}
    
// CosNaming::NamingContext interface (unsupported)
void ApplicationRegistrar_impl::rebind(const CosNaming::Name &Name, CORBA::Object_ptr obj) throw (CosNaming::NamingContext::NotFound, 
        CosNaming::NamingContext::CannotProceed, CosNaming::NamingContext::InvalidName, CosNaming::NamingContext::AlreadyBound, CORBA::SystemException) {
    this->_context->rebind(Name, obj);
    CF::Resource_var resource = ossie::corba::_narrowSafe<CF::Resource>(obj);
    if (!CORBA::is_nil(resource)) {
        _application->registerComponent(resource);
    }
}
