#ifndef CPU_THRESHOLD_MONITOR_H_
#define CPU_THRESHOLD_MONITOR_H_

#include "ThresholdMonitor.h"

class CpuUsageAccumulator;

class CpuThresholdMonitor : public GenericThresholdMonitor<float>
{
public:
	CpuThresholdMonitor( const std::string& source_id, const float* threshold, const CpuUsageAccumulator& cpu_usage_accumulator );

	static std::string GetResourceId(){ return "cpu"; }
	static std::string GetMessageClass(){ return "CPU_IDLE"; }
};

#endif
