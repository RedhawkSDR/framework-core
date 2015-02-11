#ifndef NIC_FACADE_H_
#define NIC_FACADE_H_

#include "NicInterfaceFilter.h"
#include "states/State.h"
#include "statistics/Statistics.h"
#include "reports/Reporting.h"
#include "struct_props.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <map>

class NicState;
class NicAccumulator;
class NicAllocator;

class NicFacade : public State, public Statistics, public Reporting
{
public:
    NicFacade( const double& max_throughput_percent,
               const std::vector<std::string>& nic_interface_regexes,
               std::vector<std::string>& filtered_nic_interfaces_reporting_data,
               std::vector<interfaces_struct>& reporting_data,
               std::vector<nic_metrics_struct_struct>& nic_metrics_reporting_data,
               std::vector<nic_allocation_status_struct_struct>& nic_allocation_status_reporting_data );
    
    void initialize();

    bool allocate_capacity( const nic_allocation_struct& alloc );
    void deallocate_capacity( const nic_allocation_struct& alloc );
    
    void update_state();
    void compute_statistics();
    void report();
    
    std::vector<std::string> get_devices() const;
    float get_throughput_by_device( const std::string& device ) const;
    double get_throughput_by_device_bps( const std::string& device ) const;

private:
    std::vector<std::string> poll_nic_interfaces() const;
    boost::shared_ptr<NicState> get_or_insert_nic_state( const std::string& interface );
    bool has_nic_accumulator( const std::string& device ) const;
    void add_nic_accumulator( boost::shared_ptr<NicAccumulator> nic_accumulator );
    
    void write_filtered_nic_interfaces_reporting_data();
    void write_reporting_data();
    void write_nic_metrics_reporting_data();
    void write_nic_allocation_status_reporting_data();
    
    std::string flags_to_str( unsigned int flags ) const;
    
private:
    typedef std::map<std::string, boost::shared_ptr<NicState> > NicStates;
    typedef std::map<std::string, boost::shared_ptr<NicAccumulator> > NicAccumulators;
    
    NicStates nic_states_; // Indexed by nic interface
    NicStates filtered_nic_states_;
    NicAccumulators nic_accumulators_; // Indexed by nic device (interface without vlan)
    
    NicInterfaceFilter nic_interface_filter_;
    boost::shared_ptr<NicAllocator> nic_allocator_;
    
    std::vector<std::string>& filtered_nic_interfaces_reporting_data_;
    std::vector<interfaces_struct>& reporting_data_;
    std::vector<nic_metrics_struct_struct>& nic_metrics_reporting_data_;
    std::vector<nic_allocation_status_struct_struct>& nic_allocation_status_reporting_data_;
};

#endif