#ifndef NIC_ALLOCATOR_H_
#define NIC_ALLOCATOR_H_

#include "struct_props.h"

#include <boost/function.hpp>

#include <map>
#include <stdexcept>

class NicState;

struct DeviceThroughputCapacity
{
    DeviceThroughputCapacity():
    maximum_throughput(0),
    allocated_throughput(0)
    {}
    
    std::string device;
    double maximum_throughput;
    double allocated_throughput;
};

class NicAllocator
{
public:
    class InvalidAllocation : public std::invalid_argument
    {
    public:
        explicit InvalidAllocation(const std::string& arg): std::invalid_argument(arg){};
    };
    
public:
    typedef std::map<std::string, boost::shared_ptr<NicState> > AllocatableNics;
    typedef std::map<std::string, nic_allocation_struct> Allocations;
    typedef std::map<std::string, DeviceThroughputCapacity> DeviceThroughputCapacities;
    typedef boost::function< double (const std::string&) > CurrentThroughputFunction;
    
public:
    NicAllocator( const AllocatableNics& allocatable_nics, 
                  const double& max_throughput_percent,
                  const CurrentThroughputFunction& current_throughput );
    
    bool allocate_capacity( const nic_allocation_struct& alloc );
    void deallocate_capacity( const nic_allocation_struct& alloc );
    
    const Allocations& get_allocations() const { return allocations_; }
    double get_allocated_device_throughput( const std::string& device ) const;
    
    bool allocatable_by_is_addressable( const nic_allocation_struct& alloc, const NicState& nic ) const;
    bool allocatable_by_multicast_support( const nic_allocation_struct& alloc, const NicState& nic ) const;
    bool allocatable_by_interface( const nic_allocation_struct& alloc, const NicState& nic ) const;
    bool allocatable_by_data_rate( double delta_alloc_throughput, const DeviceThroughputCapacity& capacity, double current_throughput ) const;
    
    static void MergeAllocationStructs( nic_allocation_struct& merged, const nic_allocation_struct& updated );
    
    static bool IsAddressableV4( const std::string& addr1, const std::string& addr2, const std::string& netmask );

public:
    static uint32_t GetV4AddressValue( const std::string& address );
    
private:
    void validate_allocation( const nic_allocation_struct& alloc ) const;
    
    double get_throughput(const nic_allocation_struct& alloc) const{ return alloc.data_rate*alloc.data_size; }
    DeviceThroughputCapacity& lookup_nic_throughput_capacity(const NicState& nic);
    
private:
    const AllocatableNics& allocatable_nics_;
    const double& max_throughput_percent_;
    CurrentThroughputFunction current_throughput_;
    Allocations allocations_;
    DeviceThroughputCapacities device_throughput_capacities_;
};

#endif