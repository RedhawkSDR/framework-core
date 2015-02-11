#ifndef CPU_STATE_H_
#define CPU_STATE_H_

#include "State.h"

#include <boost/shared_ptr.hpp>

#include <vector>

class ProcStatFileParser;
class ProcStatFileData;

class CpuState : public State
{
public:
    CpuState();

    void update_state();
    
    virtual const std::vector<unsigned long>& get_cpu_jiffies() const;
    virtual unsigned long get_os_start_time() const;

private:
    boost::shared_ptr<ProcStatFileData> data_;
};

#endif