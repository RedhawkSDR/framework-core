#ifndef CPU_USAGE_ACCUMULATOR_H_
#define CPU_USAGE_ACCUMULATOR_H_

#include "Statistics.h"

#include <vector>
#include <iosfwd>

#include <boost/shared_ptr.hpp>

class CpuState;

class CpuUsageAccumulator : public Statistics
{
public:
    typedef std::vector<unsigned long> CpuJiffies;

public:
    CpuUsageAccumulator(const boost::shared_ptr<const CpuState>& cpu_state);
    virtual ~CpuUsageAccumulator(){}

    virtual void compute_statistics();

    virtual unsigned long get_delta_cpu_jiffies_total() const;

    virtual double get_user_percent() const;
    virtual double get_system_percent() const;
    virtual double get_idle_percent() const;

private:
    double get_total_jiffies(const CpuJiffies& cpu_jiffies) const;

    double get_cpu_field_percent( size_t field ) const;

private:
    CpuJiffies prev_cpu_jiffies_;
    CpuJiffies current_cpu_jiffies_;
    
    boost::shared_ptr<const CpuState> cpu_state_;
};

#endif
