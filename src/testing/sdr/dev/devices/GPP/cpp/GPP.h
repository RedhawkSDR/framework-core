/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef GPP_IMPL_H
#define GPP_IMPL_H

#include "GPP_base.h"
#include <boost/shared_ptr.hpp>

#include "reports/NicThroughputThresholdMonitor.h"
#include "states/CpuState.h"
#include "statistics/CpuUsageAccumulator.h"
#include "reports/SystemMonitorReporting.h"
#include "reports/CpuThresholdMonitor.h"
#include "NicFacade.h"
#include "ossie/Events.h"

#define PROCESSOR_NAME "DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b"
#define OS_NAME "DCE:4a23ad60-0b25-4121-a630-68803a498f75"
#define OS_VERSION "DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192"

class ThresholdMonitor;
class NicFacade;

struct system_monitor_struct {
        system_monitor_struct ()
	{
	};
	
	CORBA::ULong physical_memory_free;
	double idle_cpu_percent;
};

struct component_description_struct {
    component_description_struct ()
    {
    };
    std::string appName;
    std::string identifier;
};

inline bool operator== (const component_description_struct& s1, const component_description_struct& s2) {
    if (s1.appName!=s2.appName)
        return false;
    if (s1.identifier!=s2.identifier)
        return false;
    return true;
};

class GPP_i : public GPP_base
{
    ENABLE_LOGGING
    public:
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~GPP_i();
        int serviceFunction();
        void initializeNetworkMonitor();
        void initializeCpuMonitor();
        void addThresholdMonitor( ThresholdMonitor* threshold_monitor );
        void send_threshold_event(const threshold_event_struct& message);
        
        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);
        bool allocate_loadCapacity(const double &value);
        void deallocate_loadCapacity(const double &value);
        CF::ExecutableDevice::ProcessID_Type execute (const char* name, const CF::Properties& options, const CF::Properties& parameters)
            throw (CORBA::SystemException, CF::Device::InvalidState, CF::ExecutableDevice::InvalidFunction, 
                   CF::ExecutableDevice::InvalidParameters, CF::ExecutableDevice::InvalidOptions, 
                   CF::InvalidFileName, CF::ExecutableDevice::ExecuteFail);
        void terminate (CF::ExecutableDevice::ProcessID_Type processId)
            throw (CORBA::SystemException, CF::ExecutableDevice::InvalidProcess, CF::Device::InvalidState);
        void updateThresholdMonitors();
        void calculateSystemMemoryLoading();
        std::vector<int> getPids();
        component_description_struct getComponentDescription(int pid);
        void sendChildNotification(const std::string &comp_id, const std::string &app_id);
        bool allocateCapacity_nic_allocation(const nic_allocation_struct &value);
        void deallocateCapacity_nic_allocation(const nic_allocation_struct &value);
        void deallocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);
        CORBA::Boolean allocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException);
        void releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError);
        void process_ODM(const CORBA::Any &data);

    protected:
        void updateUsageState();
        boost::shared_ptr<NicFacade> nic_facade;
        typedef std::vector<boost::shared_ptr<State> > StateSequence;
        typedef std::vector<boost::shared_ptr<Statistics> > StatisticsSequence;
        typedef std::vector<boost::shared_ptr<Reporting> > ReportingSequence;
        std::vector< boost::shared_ptr<ThresholdMonitor> > threshold_monitors;
        boost::shared_ptr<CpuState> cpu_state;
        system_monitor_struct system_monitor;
        void addPid(int pid, std::string appName, std::string identifier);
        void removePid(int pid);
        void addReservation(const component_description_struct &component);
        void removeReservation(const component_description_struct &component);
        void TableReservation(const component_description_struct &component);
        void RestoreReservation(const component_description_struct &component);
        void reservedChanged(const float *oldValue, const float *newValue);
        void establishModifiedThresholds();


        void sigchld_handler( int sig );

        std::vector<component_description_struct> reservations;
        std::vector<component_description_struct> tabled_reservations;
        std::map<int, component_description_struct> pids;
        boost::mutex pidLock;

        StateSequence states;
        StatisticsSequence statistics;
        ReportingSequence reports;
        
        std::string binary_location;
        
        thresholds_struct modified_thresholds;
        
        float idle_capacity_modifier;


        boost::posix_time::ptime   time_mark;
        
        redhawk::events::SubscriberPtr odm_consumer;
        redhawk::events::ManagerPtr mymgr;
        

 private:
        void _init();
};

#endif // GPP_IMPL_H
