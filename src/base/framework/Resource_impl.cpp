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


#include "ossie/Resource_impl.h"

PREPARE_LOGGING(Resource_impl)

Resource_impl::Resource_impl (const char* _uuid) :
    _started(false),
    component_running_mutex(),
    component_running(&component_running_mutex),
    _identifier(_uuid)
{
}


Resource_impl::Resource_impl (const char* _uuid, const char *label) :
    _started(false),
    component_running_mutex(),
    component_running(&component_running_mutex),
    _identifier(_uuid),
    naming_service_name(label)
{
}


void Resource_impl::setAdditionalParameters(std::string softwareProfile)
{
    _softwareProfile = softwareProfile;
}


void Resource_impl::start () throw (CORBA::SystemException, CF::Resource::StartError)
{
    startPorts();
    _started = true;
}


void Resource_impl::stop () throw (CORBA::SystemException, CF::Resource::StopError)
{
    stopPorts();
    _started = false;
}

char* Resource_impl::identifier () throw (CORBA::SystemException)
{
    return CORBA::string_dup(_identifier.c_str());
}

char* Resource_impl::softwareProfile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_softwareProfile.c_str());
}

CORBA::Boolean Resource_impl::started () throw (CORBA::SystemException)
{
    return _started;
}

void Resource_impl::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    releasePorts();
    
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);

    component_running.signal();
}

void Resource_impl::run() {
    // Start handling CORBA requests
    LOG_TRACE(Resource_impl, "handling CORBA requests");
    component_running.wait();
    LOG_TRACE(Resource_impl, "leaving run()");
}

void Resource_impl::halt() {
    LOG_DEBUG(Resource_impl, "Halting component")

    LOG_TRACE(Resource_impl, "Sending device running signal");
    component_running.signal();
    LOG_TRACE(Resource_impl, "Done sending device running signal");
}

void Resource_impl::setCurrentWorkingDirectory(std::string& cwd) {
    this->currentWorkingDirectory = cwd;
}

std::string& Resource_impl::getCurrentWorkingDirectory() {
    return this->currentWorkingDirectory;
}
