#ifndef NIC_THROUGHPUT_THRESHOLD_MONITOR_H_
#define NIC_THROUGHPUT_THRESHOLD_MONITOR_H_

#include "ThresholdMonitor.h"

class NicThroughputThresholdMonitor : public GenericThresholdMonitor<float, std::greater<float> >
{
public:
	NicThroughputThresholdMonitor( const std::string& source_id, const std::string& resource_id, QueryFunction threshold, QueryFunction measured );

	static std::string GetMessageClass(){ return "NIC_THROUGHPUT"; }
};

#endif
