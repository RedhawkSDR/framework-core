#include "CpuThresholdMonitor.h"
#include "../statistics/CpuUsageAccumulator.h"
#include "../utils/ReferenceWrapper.h"

class CpuUsageAccumulatorQueryFunction
{
public:
	CpuUsageAccumulatorQueryFunction( const CpuUsageAccumulator& cpu_usage_accumulator ):
	cpu_usage_accumulator_(cpu_usage_accumulator)
	{}

	float operator()() const { return cpu_usage_accumulator_.get_idle_percent(); }

private:
	const CpuUsageAccumulator& cpu_usage_accumulator_;
};

CpuThresholdMonitor::CpuThresholdMonitor( const std::string& source_id, const float* threshold, const CpuUsageAccumulator& cpu_usage_accumulator ):
GenericThresholdMonitor<float>(source_id, GetResourceId(), GetMessageClass(), MakeCref(*threshold), CpuUsageAccumulatorQueryFunction(cpu_usage_accumulator) )
{

}
