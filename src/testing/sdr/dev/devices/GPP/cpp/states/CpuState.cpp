#include "CpuState.h"
#include "../parsers/ProcStatFileParser.h"
#include "../utils/FileReader.h"

#include <sstream>

CpuState::CpuState():
data_(new ProcStatFileData())
{
    
}

void 
CpuState::update_state()
{
	std::stringstream file_str( FileReader::ReadFile( "/proc/stat" ) );
    ProcStatFileParser::Parse( file_str, *data_ );
}

const ProcStatFileData::CpuJiffies& 
CpuState::get_cpu_jiffies() const 
{ 
    return data_->cpu_jiffies; 
}

unsigned long
CpuState::get_os_start_time() const 
{ 
    return data_->os_start_time; 
}

