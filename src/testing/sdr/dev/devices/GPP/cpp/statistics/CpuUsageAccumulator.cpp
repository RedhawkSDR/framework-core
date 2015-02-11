#include "CpuUsageAccumulator.h"
#include "../states/CpuState.h"
#include "../parsers/ProcStatFileParser.h"

#include <numeric>
#include <iostream>

CpuUsageAccumulator::CpuUsageAccumulator(const boost::shared_ptr<const CpuState>& cpu_state):
cpu_state_(cpu_state)
{

}

void
CpuUsageAccumulator::compute_statistics()
{
	prev_cpu_jiffies_ = current_cpu_jiffies_;
    current_cpu_jiffies_ = cpu_state_->get_cpu_jiffies();
}

unsigned long
CpuUsageAccumulator::get_delta_cpu_jiffies_total() const
{
	if( !prev_cpu_jiffies_.empty() )
		return get_total_jiffies(current_cpu_jiffies_) - get_total_jiffies(prev_cpu_jiffies_);
	else
		return 0;
}

double
CpuUsageAccumulator::get_total_jiffies(const ProcStatFileData::CpuJiffies& cpu_jiffies) const
{
	return std::accumulate(cpu_jiffies.begin(),
	                       cpu_jiffies.end(),
	                       ProcStatFileData::CpuJiffies::value_type(0));
}

double
CpuUsageAccumulator::get_user_percent() const
{
	return get_cpu_field_percent( ProcStatFileData::CPU_JIFFIES_USER );
}

double
CpuUsageAccumulator::get_system_percent() const
{
	return get_cpu_field_percent( ProcStatFileData::CPU_JIFFIES_SYSTEM );
}

double
CpuUsageAccumulator::get_idle_percent() const
{
	return get_cpu_field_percent( ProcStatFileData::CPU_JIFFIES_IDLE );
}

double
CpuUsageAccumulator::get_cpu_field_percent( size_t field ) const
{
	double diff_total( get_delta_cpu_jiffies_total() );

	if( 0 != diff_total && field < prev_cpu_jiffies_.size() )
		return (current_cpu_jiffies_[field]-prev_cpu_jiffies_[field]) / diff_total * 100.0;
	else
		return 0;
}
