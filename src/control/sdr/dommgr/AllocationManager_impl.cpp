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


#include <string>
#include "AllocationManager_impl.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include "ossie/ossieSupport.h"

static void addPropertyRepeats(const CF::DataType& dt, CF::Properties& prop)
{
    // New id, add property at end
    unsigned int index = prop.length();
    prop.length(index + 1);
    prop[index] = dt;
}

PREPARE_LOGGING(AllocationManager_impl);

AllocationManager_impl::AllocationManager_impl (DomainManager_impl* domainManager)
{
    _domainManager = domainManager;
    this->_allocations.clear();
}

AllocationManager_impl::AllocationManager_impl (DomainManager_impl* domainManager, std::map<std::string, ossie::_allocationsType> &ref_allocations, std::map<std::string, CF::AllocationManager_var> &ref_remoteAllocations)
{
    _domainManager = domainManager;
    this->_allocations = ref_allocations;
    this->_remoteAllocations = ref_remoteAllocations;
}

AllocationManager_impl::~AllocationManager_impl ()
{
}

void AllocationManager_impl::unfilledRequests(CF::AllocationManager::AllocationRequestSequence &requests, const CF::AllocationManager::AllocationResponseSequence &result)
{
    if (requests.length() == 0) {
        return;
    }

    std::vector<unsigned int> unfilled_request_idx;
    unfilled_request_idx.resize(0);
    for (unsigned int req_idx=0; req_idx<result.length(); req_idx++) {
        std::string req_request_id = ossie::corba::returnString(requests[req_idx].requestID);
        bool found_match = false;
        for (unsigned int res_idx=0; res_idx<requests.length(); res_idx++) {
            std::string res_request_id = ossie::corba::returnString(result[res_idx].requestID);
            if (res_request_id == req_request_id) {
                found_match = true;
                break;
            }
        }
        if (not found_match) {
            unfilled_request_idx.push_back(req_idx);
        }
    }
    std::sort(unfilled_request_idx.begin(),unfilled_request_idx.end(),std::greater<unsigned int>());
    for (std::vector<unsigned int>::iterator ur_itr = unfilled_request_idx.begin(); ur_itr != unfilled_request_idx.end(); ur_itr++) {
        for (unsigned int idx=(*ur_itr); idx<requests.length()-1; idx++) {
            requests[idx] = requests[idx+1];
        }
        requests.length(requests.length()-1);
    }
    return;
}

CF::AllocationManager::AllocationResponseSequence* AllocationManager_impl::allocate(const CF::AllocationManager::AllocationRequestSequence &requests) throw (CF::AllocationManager::AllocationError)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    // try to fulfill the request locally
    CF::AllocationManager::AllocationResponseSequence_var result = this->allocateLocal(requests, this->_domainManager->getDomainManagerName().c_str());

    if (result->length() != requests.length()) {
        const ossie::DomainManagerList remoteDomains = this->_domainManager->getRegisteredRemoteDomainManagers();
        ossie::DomainManagerList::const_iterator remoteDomains_itr = remoteDomains.begin();

        CF::AllocationManager::AllocationRequestSequence remaining_requests;
        remaining_requests.length(requests.length());
        for (unsigned ridx=0;ridx<requests.length();ridx++) {
            remaining_requests[ridx] = requests[ridx];
        }

        while ((remoteDomains_itr != remoteDomains.end()) and (result->length() != requests.length())) {
            unfilledRequests(remaining_requests, result);
            CF::AllocationManager::AllocationResponseSequence_var new_result = (*remoteDomains_itr).domainManager->allocationMgr()->allocate(remaining_requests);
            if (new_result->length() != 0) {
                for (unsigned int idx=0; idx<new_result->length(); idx++) {
                    ossie::corba::push_back((CF::AllocationManager::AllocationResponseSequence&)result, new_result[idx]);
                    std::string allocid = ossie::corba::returnString(new_result[idx].allocationID);
                    _remoteAllocations[allocid] = CF::AllocationManager::_duplicate((*remoteDomains_itr).domainManager->allocationMgr());
                }
            }
            remoteDomains_itr++;
        }
        // allocateLocal updates the database, so update only if remote allocations were needed
        this->_domainManager->updateAllocations(this->_allocations, this->_remoteAllocations);
    }

    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Allocates a set of dependencies only inside the local Domain */
CF::AllocationManager::AllocationResponseSequence* AllocationManager_impl::allocateLocal(const CF::AllocationManager::AllocationRequestSequence &requests, const char* domainName) throw (CF::AllocationManager::AllocationError)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    CF::AllocationManager::AllocationResponseSequence_var result = new CF::AllocationManager::AllocationResponseSequence();
    if (requests.length() == 0) {
        result->length(0);
        return result._retn();
    }

    std::map<std::string, ossie::_allocationsType> _local_allocations;

    for (unsigned int request_idx=0; request_idx<requests.length(); request_idx++) {

        // check to make sure that the requested devices in the DAS are not nil
        if (requests[request_idx].requestedDevices.length() > 0) {
            bool at_least_one_device_is_nil = false;
            for (unsigned int i=0; i<requests[request_idx].requestedDevices.length(); i++) {
                if (CORBA::is_nil(requests[request_idx].requestedDevices[i])) {
                    at_least_one_device_is_nil = true;
                    break;
                }
            }
            if (at_least_one_device_is_nil) {
                continue;
            }
        }

        ossie::DeviceList _registeredDevices = this->_domainManager->getRegisteredDevices();
        std::vector<std::string> requestedDevices;
        for (unsigned int devsearch=0; devsearch<requests[request_idx].requestedDevices.length(); devsearch++) {
            std::string devid = ossie::corba::returnString(requests[request_idx].requestedDevices[devsearch]->identifier());
            requestedDevices.push_back(devid);
        }

        // iterate through devices and try to find suitable matches for the 'usesdevice' dependencies
        for (ossie::DeviceList::iterator node = _registeredDevices.begin(); node != _registeredDevices.end(); ++node) {
            bool deviceNotInRequestList = true;
            if (requestedDevices.size() == 0)
                deviceNotInRequestList = false;
            for (std::vector<std::string>::iterator devsearch=requestedDevices.begin(); devsearch!=requestedDevices.end(); devsearch++) {
                if (node->identifier == (*devsearch)) {
                    deviceNotInRequestList = false;
                    break;
                }
            }
            if (deviceNotInRequestList)
                continue;

            bool deviceSatisfiedRequest = false;
            if (!ossie::corba::objectExists(node->device)) {
                LOG_WARN(AllocationManager_impl, "Not using device for uses_device allocation " << node->identifier << " because it no longer exists");
                continue;
            }
            CF::Device_ptr device = CF::Device::_nil();
            try {
                if (node->device->usageState() == CF::Device::BUSY) {
                    continue;
                }
            } catch ( ... ) {
                // bad device reference or device in an unusable state
                LOG_WARN(AllocationManager_impl, " Unable to verify device's state....continuing")
                continue;
            }
            //CF::Properties allocProps;

            std::string identifier = node->identifier;
            std::string label = node->label;

            // Get the Device Manager
            CF::DeviceManager_ptr devMgr = node->devMgr.deviceManager;
            if (!ossie::corba::objectExists(devMgr)) {
                LOG_ERROR(AllocationManager_impl, "Could not locate device manager for device "<< identifier)
                std::ostringstream eout;
                eout << "Could not locate device manager for device " << identifier;
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            // Get the device manager's file system
            CF::FileSystem_var fileSystem = devMgr->fileSys();
            if (CORBA::is_nil(fileSystem)) {
                LOG_ERROR(AllocationManager_impl, "Could not locate device manager filesystem ")
                std::ostringstream eout;
                eout << "Could not locate device manager filesystem in Device Manager " << devMgr->label();
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            // parse the device's spd
            ossie::SoftPkg spd;
            try {
                CORBA::String_var profile = node->device->softwareProfile();
                File_stream _spd(fileSystem, profile);
                spd.load(_spd, static_cast<const char*>(profile));
                _spd.close();
            } catch (ossie::parser_error& e) {
                std::ostringstream eout;
                eout << "creating application error; error parsing spd; " << e.what();
                LOG_ERROR(AllocationManager_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            } catch( ... ) {
                std::ostringstream eout;
                eout << "creating application error; ; unknown error parsing spd;";
                LOG_ERROR(AllocationManager_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            // parse the device's prf
            CF::Properties allocProps;
            CF::Properties matchingAllocProps;
            if ( spd.getPRFFile() ) {

                LOG_TRACE(AllocationManager_impl, "Opening device property file" << spd.getPRFFile());
                ossie::Properties prf;
                try {
                    File_stream _prf(fileSystem, spd.getPRFFile());
                    prf.load(_prf);
                    _prf.close();
                } catch (ossie::parser_error& e) {
                    std::ostringstream eout;
                    eout << "creating application error; error parsing device prf; " << e.what();
                    LOG_ERROR(AllocationManager_impl, eout.str());
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                } catch( ... ) {
                    std::ostringstream eout;
                    eout << "creating application error; ; unknown error parsing  device prf;";
                    LOG_ERROR(AllocationManager_impl, eout.str());
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                }

                ossie::Properties additional_prf;
                //check to see if there is an implementation specific PRF
                CORBA::String_var devImplId = node->devMgr.deviceManager->getComponentImplementationId(node->identifier.c_str());
                std::vector<ossie::SPD::Implementation>::const_iterator devImpl;
                for (devImpl = spd.getImplementations().begin(); devImpl != spd.getImplementations().end();  ++devImpl) {
                    if (strcmp(devImpl->getID(), devImplId) == 0) {
                        if (devImpl->getPRFFile()) {
                            try {
                                File_stream _implPrf(fileSystem, devImpl->getPRFFile());
                                additional_prf.join(_implPrf);
                                _implPrf.close();
                            } catch (ossie::parser_error& e) {
                                std::ostringstream eout;
                                eout << "creating application error; error parsing implementation prf; " << e.what();
                                LOG_ERROR(AllocationManager_impl, eout.str());
                                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                            } catch( ... ) {
                                std::ostringstream eout;
                                eout << "creating application error; ; unknown error parsing implementation prf;";
                                LOG_ERROR(AllocationManager_impl, eout.str());
                                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                            }
                        }
                    }
                }

                prf.join(additional_prf);

                // determine whether or not the device in question has the required matching properties
                bool matched = false;
                try {
                    matched = checkDeviceMatching(prf, matchingAllocProps, requests[request_idx].allocationProperties, node->devMgr.deviceManager, node->device->softwareProfile());
                } catch ( ... ) {
                    // bad reference or remote pointer in an unusable state
                    LOG_WARN(AllocationManager_impl, " Attempt to check matching properties raised an exception. A device or device manager reference is bad.")
                }
                if (matched) {
                    if (matchingAllocProps.length()>0) {
                        std::string allocationID = ossie::corba::returnString(requests[request_idx].requestID);
                        CF::Properties tmp_props;
                        tmp_props.length(0);
                        ossie::corba::push_back((CF::AllocationManager::AllocationResponseSequence&)result, ossie::assembleResponse(ossie::corba::returnString(requests[request_idx].requestID),allocationID,
                                tmp_props,node->device,node->devMgr.deviceManager));
                        deviceSatisfiedRequest = true;
                    }
                } else {
                    continue;
                }

                // generate properties for the allocation call
                bool props_alloc = checkDevicePropertyTypes(prf, allocProps, requests[request_idx].allocationProperties);
                if (not props_alloc) {
                    continue;
                }

                LOG_TRACE(AllocationManager_impl, "Allocating Properties of length " << allocProps.length())
                // If there are duplicates in the allocation sequence, break up the allocation into multiple calls
                //
                //
                std::vector<unsigned int> props_to_pop;
                std::vector<CF::Properties> allocs;
                CF::Properties original_allocs = allocProps;
                allocs.resize(0);

                unsigned int outer=0;
                while (outer != allocProps.length()) {
                    std::string outer_id = ossie::corba::returnString(allocProps[outer].id);
                    std::vector<unsigned int> props_idx_match;
                    props_idx_match.resize(0);
                    for (unsigned int inner=outer+1; inner<allocProps.length(); inner++) {
                        std::string inner_id = ossie::corba::returnString(allocProps[inner].id);
                        if (inner_id == outer_id) {
                            props_idx_match.push_back(inner);
                        }
                    }
                    if (props_idx_match.size() != 0) {
                        if (allocs.size()<props_idx_match.size())
                            allocs.resize(props_idx_match.size());
                        for (unsigned int idx=0; idx<props_idx_match.size(); idx++) {
                            ossie::corba::push_back(allocs[idx], allocProps[props_idx_match[idx]]);
                        }
                        for (int idx=props_idx_match.size()-1; idx>=0; idx--) {
                            for (unsigned int alloc_idx=idx; alloc_idx<(allocProps.length()-1); alloc_idx++) {
                                allocProps[alloc_idx] = allocProps[alloc_idx+1];
                            }
                            allocProps.length(allocProps.length()-1);
                        }
                    } else {
                        outer++;
                    }
                }

                try {
                    if (allocProps.length() > 0) {
                        if (!this->completeAllocations(node->device, allocProps,allocs)) {
                            if (deviceSatisfiedRequest) {
                                if (result->length()>0)
                                    result->length(result->length()-1);
                            }
                            LOG_TRACE(AllocationManager_impl, "Device " << identifier << " " << label << " lacks sufficient capacity")
                            continue;
                        }
                        LOG_TRACE(AllocationManager_impl, "The outgoing allocProps length is " << original_allocs.length());
                    } else {
                        LOG_TRACE(AllocationManager_impl, "component 'usesdevice' requires no capacity from device " << label)
                    }
                    std::string allocationID = ossie::corba::returnString(requests[request_idx].requestID);
                    if (deviceSatisfiedRequest) {
                        result[result->length()-1].allocatedDevice = CF::Device::_duplicate(node->device);
                        result[result->length()-1].allocationDeviceManager = CF::DeviceManager::_duplicate(node->devMgr.deviceManager);
                        result[result->length()-1].requestID = requests[request_idx].requestID;
                        result[result->length()-1].allocationID = CORBA::string_dup(allocationID.c_str());
                        for (unsigned int alloc_prop_idx=0; alloc_prop_idx<original_allocs.length(); alloc_prop_idx++) {
                            ossie::corba::push_back(result[result->length()-1].allocationProperties, original_allocs[alloc_prop_idx]);
                        }
                        deviceSatisfiedRequest = true;
                    } else {
                        ossie::corba::push_back((CF::AllocationManager::AllocationResponseSequence&)result, ossie::assembleResponse(ossie::corba::returnString(requests[request_idx].requestID),allocationID,
                                original_allocs,node->device,node->devMgr.deviceManager));
                        deviceSatisfiedRequest = true;
                    }
                    _local_allocations[allocationID].allocatedDevice = ossie::corba::objectToString(node->device);
                    _local_allocations[allocationID].allocationDeviceManager = ossie::corba::objectToString(node->devMgr.deviceManager);
                    _local_allocations[allocationID].allocationProperties = original_allocs;
                    _local_allocations[allocationID].requestingDomain = this->_domainManager->getDomainManagerName();
                    device = node->device;
                    break;
                } catch (CF::Device::InvalidCapacity e) {
                    LOG_TRACE(AllocationManager_impl, "Device " << identifier << " " << label << " lacks sufficient capacity to satisfy 'usesdevice'")
                        continue;
                } catch (CF::Device::InsufficientCapacity e) {
                    LOG_TRACE(AllocationManager_impl, "Device " << identifier << " " << label << " lacks sufficient capacity to satisfy 'usesdevice'")
                        continue;
                }
            }
        }
    }
    
    std::map<std::string, ossie::_allocationsType>::iterator _localall_iter = _local_allocations.begin();
    while (_localall_iter != _local_allocations.end()){
        this->_allocations[(*_localall_iter).first] = _local_allocations[(*_localall_iter).first];
        _localall_iter++;
    }

    if (result->length() != 0) {
        this->_domainManager->updateAllocations(this->_allocations, this->_remoteAllocations);
    }
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

bool AllocationManager_impl::completeAllocations(CF::Device_ptr device, CF::Properties &allocProps, std::vector<CF::Properties>& duplicates)
{
    bool retval = false;
    try {
        retval = device->allocateCapacity (allocProps);
    } catch ( ... ) {
        // bad device reference, device in an unusable state, or bad property set
        LOG_WARN(AllocationManager_impl, " Allocation raised an exception.")
    }
    if (not retval) {
        return retval;
    }
    unsigned int dup_idx = 0;
    while (dup_idx != duplicates.size()) {
        retval = false;
        try {
            retval = device->allocateCapacity(duplicates[dup_idx]);
        } catch ( ... ) {
            // bad device reference, device in an unusable state, or bad property set
            LOG_WARN(AllocationManager_impl, " Allocation raised an exception.")
        }
        if (not retval) {
            try {
                device->deallocateCapacity(allocProps);
            } catch ( ... ) {
                // bad device reference, device in an unusable state, or bad property set
                LOG_WARN(AllocationManager_impl, " Deallocation raised an exception.")
            }
            for (unsigned int clean_idx=0; clean_idx<dup_idx; clean_idx++) {
                try {
                    device->deallocateCapacity(duplicates[clean_idx]);
                } catch ( ... ) {
                    // bad device reference, device in an unusable state, or bad property set
                    LOG_WARN(AllocationManager_impl, " Deallocation raised an exception.")
                }
            }
            return retval;
        }
        dup_idx++;
    }
    return retval;
}

bool AllocationManager_impl::checkDeviceMatching(ossie::Properties& _prf, CF::Properties& propertiesToCheckForMatch, const CF::Properties& dependencyPropertiesFromComponent,
        CF::DeviceManager_ptr _devMgr, const char *_deviceSoftwareProfile)
{
    propertiesToCheckForMatch.length(0);
    std::string processor("DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b");
    std::string operating_system("DCE:4a23ad60-0b25-4121-a630-68803a498f75");
    std::string operating_system_version("DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192");

    unsigned int matching_processor = 0;
    unsigned int matching_operating_system = 0;
    unsigned int matching_operating_system_version = 0;

    ossie::DeviceManagerConfiguration dcdParser;
    std::string deviceManagerProfile = "";

    const std::vector<const ossie::Property*>& allocationProps = _prf.getAllocationProperties();
    for (unsigned int compprop_idx=0; compprop_idx<dependencyPropertiesFromComponent.length(); compprop_idx++) {

        std::string comp_propid = ossie::corba::returnString(dependencyPropertiesFromComponent[compprop_idx].id);

        const ossie::Property* matchingProp = 0;
        for (std::vector<const ossie::Property*>::const_iterator devprop_iter=allocationProps.begin(); devprop_iter!=allocationProps.end(); devprop_iter++) {
            std::string dev_propid = ossie::corba::returnString((*devprop_iter)->getID());
            if (dev_propid == comp_propid) {
                matchingProp = (*devprop_iter);
                break;
            }
        }

        if (matchingProp) {
            // this function deals only with non-external properties
            if (matchingProp->isExternal()) {
                continue;
            }
            LOG_DEBUG(AllocationManager_impl, " It's a matching prop");
            CF::DataType depProp = dependencyPropertiesFromComponent[compprop_idx];
            const ossie::SimpleProperty* simpleProp = dynamic_cast<const ossie::SimpleProperty*>(matchingProp);
            if (!simpleProp) {
                LOG_ERROR(AllocationManager_impl, "Invalid action '" << matchingProp->getAction()
                          << "' for non-simple property " << comp_propid);
                return false;
            }
            std::string action = simpleProp->getAction();

            LOG_TRACE(AllocationManager_impl, comp_propid << "=" << ossie::corba::returnString(simpleProp->getID()) << " value "
                      << ossie::corba::returnString(simpleProp->getValue()) << " " << action << " "
                      << ossie::any_to_string(depProp.value));
            const char *propvalue;
            depProp.value >>= propvalue;

            std::string tmp_propvalue(propvalue);
            CF::DataType allocProp = ossie::convertPropertyToDataType(simpleProp);
            CORBA::Any allocValue = ossie::string_to_any(tmp_propvalue, allocProp.value.type());
            depProp.value = allocValue;

            std::string matchingpropvalue;
            if (deviceManagerProfile == "") {
                deviceManagerProfile = CORBA::string_dup(_devMgr->deviceConfigurationProfile());
                File_stream _dcd(_devMgr->fileSys(), deviceManagerProfile.c_str());
                dcdParser.load(_dcd);
                _dcd.close();
                std::vector<ossie::ComponentFile> componentFiles = dcdParser.getComponentFiles();
                std::vector<ossie::ComponentFile>::iterator p = componentFiles.begin();
                std::string placement_id = "";
                while (p != componentFiles.end()) {
                    if ((*p).filename == _deviceSoftwareProfile) {
                        placement_id = (*p).id;
                        break;
                    }
                    p++;
                }
                std::vector<ossie::ComponentPlacement> componentPlacements = dcdParser.getComponentPlacements();
                std::vector<ossie::ComponentPlacement>::iterator pl = componentPlacements.begin();
                std::vector<ossie::ComponentProperty*> properties;
                properties.resize(0);
                while (pl != componentPlacements.end()) {
                    if ((*pl)._componentFileRef == placement_id) {
                        properties = (*pl).instantiations[0].properties;
                        break;
                    }
                    pl++;
                }
                std::vector<ossie::ComponentProperty*>::iterator p_prop = properties.begin();
                while (p_prop != properties.end()) {
                    if ((*p_prop)->_id == comp_propid) {
                        const ossie::SimplePropertyRef* overload = dynamic_cast<const ossie::SimplePropertyRef*>(*p_prop);
                        matchingpropvalue = overload->getValue();
                        CORBA::Any allocValue = ossie::string_to_any(matchingpropvalue, allocProp.value.type());
                        allocProp.value = allocValue;
                        break;
                    }
                    p_prop++;
                }
            }


            // Per section D.4.1.1.7 the allocation property is on the left side of the action
            // and the dependency value is on the right side of the action
            bool result = ossie::compare_anys(allocProp.value, depProp.value, action);
            if (not result) {
                if (comp_propid == processor) {
                    if (matching_processor == 0)
                        matching_processor = 2;
                    continue;
                }
                if (comp_propid == operating_system) {
                    if (matching_operating_system == 0)
                        matching_operating_system = 2;
                    continue;
                }
                if (comp_propid == operating_system_version) {
                    if (matching_operating_system_version == 0)
                        matching_operating_system_version = 2;
                    continue;
                }
                return false;
            } else {
                if (comp_propid == processor) {
                       matching_processor = 1;
                }
                if (comp_propid == operating_system) {
                       matching_operating_system = 1;
                }
                if (comp_propid == operating_system_version) {
                    matching_operating_system_version = 1;
                }
                CF::DataType tmp_prop;
                tmp_prop.id=CORBA::string_dup(matchingProp->getID());
                ossie::corba::push_back(propertiesToCheckForMatch,tmp_prop);
            }
        } else {
            // trying to compare against processor/os not available in the device. No definition in device means that it doesn't matter, so it's ok
            if ((comp_propid == processor) or (comp_propid == operating_system) or (comp_propid == operating_system_version)) {
                continue;
            }
            LOG_TRACE(AllocationManager_impl, " It's not a matching prop")
            return false;
        }
    }
    if ((matching_processor == 2) or (matching_operating_system == 2) or (matching_operating_system_version == 2))
        return false;
    return true;
}

bool AllocationManager_impl::checkDevicePropertyTypes(ossie::Properties& _prf, CF::Properties& propertiesForAllocateCall, const CF::Properties& dependencyPropertiesFromComponent)
{
    std::string processor("DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b");
    std::string operating_system("DCE:4a23ad60-0b25-4121-a630-68803a498f75");
    std::string operating_system_version("DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192");

    const std::vector<const ossie::Property*>& allocationProps = _prf.getAllocationProperties();
    for (unsigned int compprop_idx=0; compprop_idx<dependencyPropertiesFromComponent.length(); compprop_idx++) {

        std::string comp_propid = ossie::corba::returnString(dependencyPropertiesFromComponent[compprop_idx].id);

        const ossie::Property* matchingProp = 0;
        for (std::vector<const ossie::Property*>::const_iterator devprop_iter=allocationProps.begin(); devprop_iter!=allocationProps.end(); devprop_iter++) {
            std::string dev_propid = (*devprop_iter)->getID();
            if (comp_propid == operating_system) {
                if ((dev_propid == "os_name") or (dev_propid == operating_system)) {
                    matchingProp = (*devprop_iter);
                    break;
                }
            } else if (comp_propid == operating_system_version) {
                if ((dev_propid == "os_version") or (dev_propid == operating_system_version)) {
                    matchingProp = (*devprop_iter);
                    break;
                }
            } else if (comp_propid == processor) {
                if ((dev_propid == "processor_name") or (dev_propid == processor)) {
                    matchingProp = (*devprop_iter);
                    break;
                }
            } else {
                if (dev_propid == comp_propid) {
                    matchingProp = (*devprop_iter);
                    break;
                }
            }
        }

        if (matchingProp) {
            // this function deals only with external properties
            if (not matchingProp->isExternal()) {
                continue;
            }
            const ossie::SimpleProperty* simpleMatchingProp = dynamic_cast<const ossie::SimpleProperty*>(matchingProp);
            const ossie::SimpleSequenceProperty* simpleSequenceMatchingProp = dynamic_cast<const ossie::SimpleSequenceProperty*>(matchingProp);
            const ossie::StructProperty* structMatchingProp = dynamic_cast<const ossie::StructProperty*>(matchingProp);
            const ossie::StructSequenceProperty* structSequenceMatchingProp = dynamic_cast<const ossie::StructSequenceProperty*>(matchingProp);
            if (simpleMatchingProp != NULL) {
                // The depProp is the  property that is defined in the components SPD
                std::string str_propvalue = ossie::any_to_string(dependencyPropertiesFromComponent[compprop_idx].value);
                const char* propvalue = str_propvalue.c_str();
                CF::DataType depProp = dependencyPropertiesFromComponent[compprop_idx];
                LOG_TRACE(AllocationManager_impl, " Matched! " << depProp.id << " value " << propvalue)
                CORBA::Any capacityDep = ossie::string_to_any(propvalue, ossie::getTypeCode(simpleMatchingProp->getType()));

                depProp.value = capacityDep;
                LOG_TRACE(AllocationManager_impl, "Adding dependency " << depProp.id << " to be " << ossie::any_to_string(depProp.value))
                addPropertyRepeats(depProp, propertiesForAllocateCall);
            } else if (simpleSequenceMatchingProp != NULL) {
            } else if (structMatchingProp != NULL) {
                CF::DataType depProp;
                depProp.id = CORBA::string_dup(dependencyPropertiesFromComponent[compprop_idx].id);
                CF::Properties *depProps;
                if (!(dependencyPropertiesFromComponent[compprop_idx].value>>=depProps)) {
                }
                CF::Properties tmp_props;
                std::vector<ossie::SimpleProperty> structval = structMatchingProp->getValue();
                for (unsigned int i=0; i<depProps->length(); i++) {
                    std::string str_propvalue = ossie::any_to_string((*depProps)[i].value);
                    const char* propvalue = str_propvalue.c_str();
                    tmp_props.length(tmp_props.length()+1);
                    tmp_props[tmp_props.length()-1].id = CORBA::string_dup((*depProps)[i].id);
                    std::string propid = ossie::corba::returnString((*depProps)[i].id);
                    for (unsigned int j=0; j<structval.size(); j++) {
                        std::string structvalid = structval[j].getID();
                        if (propid == structvalid) {
                            tmp_props[tmp_props.length()-1].value = ossie::string_to_any(propvalue, ossie::getTypeCode(structval[j].getType()));
                            break;
                        }
                    }
                }
                depProp.value <<= tmp_props;
                addPropertyRepeats(depProp, propertiesForAllocateCall);
            } else if (structSequenceMatchingProp != NULL) {
            }
        } else {
            LOG_TRACE(AllocationManager_impl, " It's not a matching prop")
            return false;
        }
    }
    return true;
}

/* Deallocates a set of allocations */
void AllocationManager_impl::deallocate(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId)
{
    CF::AllocationManager::allocationIDSequence invalidAllocations;
    invalidAllocations.length(0);
    for (unsigned int dealloc=0; dealloc<allocationIDs.length(); dealloc++) {
        std::string allocation_id = ossie::corba::returnString(allocationIDs[dealloc]);
        std::map<std::string, ossie::_allocationsType>::iterator alloc = this->_allocations.find(allocation_id);
        if (alloc != this->_allocations.end()) {
            ossie::_allocationsType localAlloc = this->_allocations[allocation_id];
            CORBA::Object_var obj = ::ossie::corba::stringToObject(localAlloc.allocatedDevice);
            CF::Device_ptr dev = ::ossie::corba::_narrowSafe<CF::Device>(obj);
            if (!ossie::corba::objectExists(dev)) {
                LOG_WARN(AllocationManager_impl, "Not deallocating capacity a device because it no longer exists");
            } else {
                try {
                    dev->deallocateCapacity(localAlloc.allocationProperties);
                } catch ( ... ) {
                    LOG_WARN(AllocationManager_impl, " Deallocation failed....continuing")
                }
            }
            this->_allocations.erase(alloc);
        } else {
            std::map<std::string, CF::AllocationManager_var>::iterator alloc = this->_remoteAllocations.find(allocation_id);
            if (alloc != this->_remoteAllocations.end()) {
                CF::AllocationManager::allocationIDSequence allocations;
                allocations.length(1);
                allocations[0] = CORBA::string_dup(allocation_id.c_str());
                this->_remoteAllocations[allocation_id]->deallocate(allocations);
                this->_remoteAllocations.erase(alloc);
            } else {
                invalidAllocations.length(invalidAllocations.length()+1);
                invalidAllocations[invalidAllocations.length()-1] = CORBA::string_dup(allocation_id.c_str());
            }
        }
    }
    this->_domainManager->updateAllocations(this->_allocations, this->_remoteAllocations);
    if (invalidAllocations.length() != 0) {
        throw CF::AllocationManager::InvalidAllocationId(invalidAllocations);
    }
}

/* Returns all current allocations on all Domains */
CF::AllocationManager::AllocationStatusSequence* AllocationManager_impl::allocations(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    CF::AllocationManager::AllocationStatusSequence_var result = new CF::AllocationManager::AllocationStatusSequence();
    result->length(this->_allocations.size());
    std::map<std::string, ossie::_allocationsType>::iterator _alloc_iter = this->_allocations.begin();
    unsigned int idx = 0;
    while (_alloc_iter != this->_allocations.end()){
        CORBA::Object_var obj = ::ossie::corba::stringToObject((*_alloc_iter).second.allocatedDevice);
        result[idx].allocatedDevice = CF::Device::_duplicate(::ossie::corba::_narrowSafe<CF::Device>(obj));
        obj = ::ossie::corba::stringToObject((*_alloc_iter).second.allocationDeviceManager);
        result[idx].allocationDeviceManager = CF::DeviceManager::_duplicate(::ossie::corba::_narrowSafe<CF::DeviceManager>(obj));
        result[idx].allocationID = CORBA::string_dup((*_alloc_iter).first.c_str());
        result[idx].allocationProperties = (*_alloc_iter).second.allocationProperties;
        result[idx].requestingDomain = CORBA::string_dup((*_alloc_iter).second.requestingDomain.c_str());
        _alloc_iter++;
        idx++;
    }
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all current allocations that were made through the Allocation Manager that have not been deallocated */
CF::AllocationManager::AllocationStatusSequence* AllocationManager_impl::localAllocations(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    CF::AllocationManager::AllocationStatusSequence_var result = new CF::AllocationManager::AllocationStatusSequence();
    result->length(0);
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices in all Domains that can be seen by any Allocation Manager seen by the local Allocation Manager */
CF::AllocationManager::DeviceLocationSequence* AllocationManager_impl::allDevices()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    CF::AllocationManager::DeviceLocationSequence_var result = new CF::AllocationManager::DeviceLocationSequence();
    result->length(0);
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices after policy is applied by any Allocation Manager seen by the local Allocation Manager */
CF::AllocationManager::DeviceLocationSequence* AllocationManager_impl::authorizedDevices()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    CF::AllocationManager::DeviceLocationSequence_var result = new CF::AllocationManager::DeviceLocationSequence();
    result->length(0);
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices that are located within the local Domain */
CF::AllocationManager::DeviceLocationSequence* AllocationManager_impl::localDevices()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    CF::AllocationManager::DeviceLocationSequence_var result = new CF::AllocationManager::DeviceLocationSequence();
    result->length(0);

    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices that are located within the local Domain */
CF::DomainManager_ptr AllocationManager_impl::domainMgr()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    TRACE_EXIT(AllocationManager_impl)
    return CF::DomainManager::_duplicate(this->domainMgr());
}
