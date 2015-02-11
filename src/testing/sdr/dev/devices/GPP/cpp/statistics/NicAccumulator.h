#ifndef NIC_ACCUMULATOR_H_
#define	NIC_ACCUMULATOR_H_

#include "Statistics.h"

#include <vector>
#include <string>

#include <boost/function.hpp>

#include <stdint.h>

class NicState;

class NicAccumulator : public Statistics
{
public:
    typedef boost::function< double() > CurrentTimeFunction;
    typedef std::vector<std::string> Vlans;

public:
    NicAccumulator();
    
    void add_nic( const boost::shared_ptr<const NicState>& nic_state );

    void compute_statistics();

    std::string get_device() const;
    const Vlans& get_vlans() const { return vlans_; }
    std::string get_vlans_string() const { return vlans_string_; }
    double get_throughput_MB_per_sec() const { return throughput_; }

    void set_current_time_function( CurrentTimeFunction current_time );
    static double CurrentTime();

private:
    void validate_device( const std::string& device ) const;
    void add_vlan( const std::string& vlan );

private:
    typedef std::vector<boost::shared_ptr<const NicState> > NicStates;
    
    uint64_t prev_rx_bytes_;
    uint64_t prev_tx_bytes_;
    double prev_time_;
    double throughput_;

    NicStates nic_states_;
    Vlans vlans_;
    std::string vlans_string_;

    CurrentTimeFunction current_time_;
};


#endif	

