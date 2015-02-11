#ifndef SYSTEM_MONITOR_REPORTING_H_
#define SYSTEM_MONITOR_REPORTING_H_

#include "Reporting.h"

#include <boost/shared_ptr.hpp>

#include <vector>

class CpuUsageAccumulator;
class system_monitor_struct;

class SystemMonitorReporting : public Reporting
{
public:
    SystemMonitorReporting( const boost::shared_ptr<const CpuUsageAccumulator>& cpu_usage_accumulator,
                         system_monitor_struct& reporting_data);
    
    void report();
    
private:
    std::string format_up_time(unsigned long secondsUp) const;
    
private:
    boost::shared_ptr<const CpuUsageAccumulator> cpu_usage_accumulator_;
    system_monitor_struct& reporting_data_;
};


#endif