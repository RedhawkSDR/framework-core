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


#ifndef __ALLOCATIONMANAGER__IMPL
#define __ALLOCATIONMANAGER__IMPL

#include <string>
#include <list>
#include <sstream>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>
#include <ossie/FileStream.h>
#include <ossie/prop_utils.h>
#include <ossie/DeviceManagerConfiguration.h>
#include "DomainManager_impl.h"

class AllocationManager_impl: public virtual POA_CF::AllocationManager
{
    ENABLE_LOGGING
    
    public:
        AllocationManager_impl (DomainManager_impl* domainManager);
        AllocationManager_impl (DomainManager_impl* domainManager, std::map<std::string, ossie::_allocationsType> &ref_allocations, std::map<std::string, CF::AllocationManager_var> &ref_remoteAllocations);
        ~AllocationManager_impl ();
        
        CF::AllocationManager::AllocationResponseSequence* allocate(const CF::AllocationManager::AllocationRequestSequence &requests) throw (CF::AllocationManager::AllocationError);
        
        /* Allocates a set of dependencies only inside the local Domain */
        CF::AllocationManager::AllocationResponseSequence* allocateLocal(const CF::AllocationManager::AllocationRequestSequence &requests, const char* domainName) throw (CF::AllocationManager::AllocationError);
        
        /* Deallocates a set of allocations */
        void deallocate(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId);
        
        /* Returns all current allocations on all Domains */
        CF::AllocationManager::AllocationStatusSequence* allocations(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId);
        
        /* Returns all current allocations that were made through the Allocation Manager that have not been deallocated */
        CF::AllocationManager::AllocationStatusSequence* localAllocations(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId);
        
        /* Returns all devices in all Domains that can be seen by any Allocation Manager seen by the local Allocation Manager */
        CF::AllocationManager::DeviceLocationSequence* allDevices();
        
        /* Returns all devices after policy is applied by any Allocation Manager seen by the local Allocation Manager */
        CF::AllocationManager::DeviceLocationSequence* authorizedDevices();
        
        /* Returns all devices that are located within the local Domain */
        CF::AllocationManager:: DeviceLocationSequence* localDevices();
        
        /* Returns all devices that are located within the local Domain */
        CF::DomainManager_ptr domainMgr();

        // local support functions
        bool checkDeviceMatching(ossie::Properties& _prf, CF::Properties& propertiesToCheckForMatch, const CF::Properties& dependencyPropertiesFromComponent,
                CF::DeviceManager_ptr _devMgr, const char *_deviceSoftwareProfile);
        bool checkDevicePropertyTypes(ossie::Properties& _prf, CF::Properties& propertiesForAllocateCall, const CF::Properties& dependencyPropertiesFromComponent);
        bool completeAllocations(CF::Device_ptr device, CF::Properties &allocProps, std::vector<CF::Properties>& duplicates);

    private:
        DomainManager_impl* _domainManager;
        std::map<std::string, ossie::_allocationsType> _allocations;
        std::map<std::string, CF::AllocationManager_var> _remoteAllocations;
        void unfilledRequests(CF::AllocationManager::AllocationRequestSequence &requests, const CF::AllocationManager::AllocationResponseSequence &result);
    
    protected:
        boost::recursive_mutex allocationAccess;
        
};                  /* END CLASS DEFINITION AllocationManager */
#endif              /* __ALLOCATIONMANAGER__IMPL */
