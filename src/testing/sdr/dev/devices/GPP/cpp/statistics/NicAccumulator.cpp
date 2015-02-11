#include "NicAccumulator.h"
#include "../states/NicState.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <numeric>

#include <sys/time.h>

static const double BYTES_PER_MEGABYTE = 1024*1024;

NicAccumulator::NicAccumulator():
prev_rx_bytes_(0),
prev_tx_bytes_(0),
prev_time_(std::numeric_limits<double>::max()),
throughput_(0),
current_time_(CurrentTime)
{
}

void
NicAccumulator::add_nic( const boost::shared_ptr<const NicState>& nic_state )
{
    validate_device( nic_state->get_device() );
    
    nic_states_.push_back( nic_state );
    
    add_vlan( nic_state->get_vlan() );
}

void 
NicAccumulator::validate_device( const std::string& device ) const
{
    if( !get_device().empty() )
    {
        if( device != get_device() )
        {
            std::stringstream errstr;
            errstr << "Nic device name mismatch (expected=" << get_device()
                   << " actual=" << device << ")";
            throw std::logic_error(errstr.str());
        }
    }
}

void 
NicAccumulator::add_vlan( const std::string& vlan )
{
    if( !vlan.empty() )
    {
        vlans_.push_back(vlan);
        
        if( !vlans_string_.empty() )
            vlans_string_ += ", ";
        vlans_string_ += vlan;
    }
}

void NicAccumulator::compute_statistics()
{
	double current_time = current_time_();
	double delta_time = current_time - prev_time_;

    double current_rx_bytes = 0;
    double current_tx_bytes = 0;
    
    for( NicStates::const_iterator i=nic_states_.begin(); i!=nic_states_.end(); ++i )
    {
        // Ignore vlans when computing statistics
        if( (*i)->get_vlan().empty() )
        {
            current_rx_bytes += (*i)->get_rx_bytes();
            current_tx_bytes += (*i)->get_tx_bytes();
        }
    }
    
	if( delta_time > 0 )
	{
        double delta_rx_bytes = current_rx_bytes - prev_rx_bytes_;
        double delta_tx_bytes = current_tx_bytes - prev_tx_bytes_;
		throughput_ = (delta_rx_bytes + delta_tx_bytes) / (delta_time * BYTES_PER_MEGABYTE);
	}

    prev_rx_bytes_ = current_rx_bytes;
    prev_tx_bytes_ = current_tx_bytes;
	prev_time_ = current_time;
}

std::string
NicAccumulator::get_device() const
{
    if( nic_states_.empty() )
        return "";
    else
        return nic_states_[0]->get_device();
}

void
NicAccumulator::set_current_time_function( NicAccumulator::CurrentTimeFunction current_time )
{
	current_time_ = current_time;
}

double
NicAccumulator::CurrentTime()
{
    struct timeval tmp_time;
    struct timezone tmp_tz;
    gettimeofday(&tmp_time, &tmp_tz);
    return (double)tmp_time.tv_sec + (double)tmp_time.tv_usec*1e-6;
}

