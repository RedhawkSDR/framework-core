#ifndef PROC_STAT_FILE_PARSER_H_
#define PROC_STAT_FILE_PARSER_H_

#include "Parser.h"

#include <iosfwd>
#include <string>
#include <vector>

struct ProcStatFileData
{
    typedef std::vector<unsigned long> CpuJiffies;
    
    enum CpuJiffiesField
    {
        CPU_JIFFIES_USER = 0,
        CPU_JIFFIES_NICE,
        CPU_JIFFIES_SYSTEM,
        CPU_JIFFIES_IDLE,
        CPU_JIFFIES_IOWAIT,     // Since Linux 2.5.41
        CPU_JIFFIES_IRQ,        // Since Linux 2.6.0-test4
        CPU_JIFFIES_SOFTIRQ,    // Since Linux 2.6.0-test4
        CPU_JIFFIES_STEAL,      // Since Linux 2.6.11
        CPU_JIFFIES_GUEST,      // Since Linux 2.6.24
        CPU_JIFFIES_GUEST_NICE, // Since Linux 2.6.33
        CPU_JIFFIES_MAX_SIZE
    };
    
    ProcStatFileData():
    os_start_time(0),
    cpu_jiffies(CPU_JIFFIES_MAX_SIZE,0)
    {
    }
    
    unsigned long os_start_time;
    CpuJiffies cpu_jiffies;
};

class ProcStatFileParser : public OverridableSingleton<ProcStatFileParser>
{
public:
    typedef ProcStatFileData DataType;
    
public:
    virtual ~ProcStatFileParser(){}
    static void Parse( std::istream& istr, ProcStatFileData& data );

protected:
    virtual void parse( std::istream& istr, ProcStatFileData& data );
    
private:
    void reset_fields(ProcStatFileData& data);

    void parse_fields(std::istream& istr, ProcStatFileData& data);
    void parse_line(const std::vector<std::string>& values, ProcStatFileData& data);

    void validate_fields(const ProcStatFileData& data) const;
    void validate_field( bool success, const std::string& message ) const;
};

#endif
