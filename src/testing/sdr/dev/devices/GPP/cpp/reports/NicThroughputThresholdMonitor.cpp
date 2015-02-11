#include "NicThroughputThresholdMonitor.h"
#include "../utils/ReferenceWrapper.h"

NicThroughputThresholdMonitor::NicThroughputThresholdMonitor( const std::string& source_id, const std::string& resource_id, NicThroughputThresholdMonitor::QueryFunction threshold, NicThroughputThresholdMonitor::QueryFunction measured ):
GenericThresholdMonitor<float, std::greater<float> >(source_id, resource_id, GetMessageClass(), threshold, measured )
{
}
