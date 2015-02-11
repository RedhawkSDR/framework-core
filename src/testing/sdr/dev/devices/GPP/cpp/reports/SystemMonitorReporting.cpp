#include "SystemMonitorReporting.h"
#include "../statistics/CpuUsageAccumulator.h"
#include "../GPP.h"

#include <sstream>

#include <sys/sysinfo.h>

static const size_t BYTES_PER_MEGABYTE = 1024*1024;

SystemMonitorReporting::SystemMonitorReporting( const boost::shared_ptr<const CpuUsageAccumulator>& cpu_usage_accumulator,
                     system_monitor_struct& reporting_data):
cpu_usage_accumulator_(cpu_usage_accumulator),
reporting_data_(reporting_data)
{
    
}

void
SystemMonitorReporting::report()
{
	struct sysinfo info;
	sysinfo(&info);

	//reporting_data_.virtual_memory_total = (info.totalram+info.totalswap) / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.virtual_memory_free = (info.freeram+info.freeswap) / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.virtual_memory_used = reporting_data_.virtual_memory_total-reporting_data_.virtual_memory_free;
	//reporting_data_.virtual_memory_percent = (double)reporting_data_.virtual_memory_used / (double)reporting_data_.virtual_memory_total * 100.;
	//reporting_data_.physical_memory_total = info.totalram / BYTES_PER_MEGABYTE * info.mem_unit;
	reporting_data_.physical_memory_free = info.freeram / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.physical_memory_used = reporting_data_.physical_memory_total-reporting_data_.physical_memory_free;
	//reporting_data_.physical_memory_percent = (double)reporting_data_.physical_memory_used / (double)reporting_data_.physical_memory_total * 100.;
	//reporting_data_.user_cpu_percent = cpu_usage_accumulator_->get_user_percent();
	//reporting_data_.system_cpu_percent = cpu_usage_accumulator_->get_system_percent();
	reporting_data_.idle_cpu_percent = cpu_usage_accumulator_->get_idle_percent();
	//reporting_data_.cpu_percent = 100.0 - reporting_data_.idle_cpu_percent;
	//reporting_data_.up_time = info.uptime;
	//reporting_data_.up_time_string = format_up_time(reporting_data_.up_time);
	//reporting_data_.last_update_time = time(NULL);
}

std::string
SystemMonitorReporting::format_up_time(unsigned long secondsUp) const
{
	std::stringstream formattedUptime;
	int days;
	int hours;
	int minutes;
	int seconds;

	int leftover;

	days = (int) secondsUp / (60 * 60 * 24);
	leftover = (int) secondsUp - (days * (60 * 60 * 24) );
	hours = (int) leftover / (60 * 60);
	leftover = leftover - (hours * (60 * 60) );
	minutes = (int) leftover / 60;
	seconds = leftover - (minutes * 60);

	formattedUptime << days << "d " << hours << "h " << minutes << "m " << seconds << "s";

	return formattedUptime.str();
}
