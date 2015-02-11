#include "CmdlineExecutor.h"

#include <stdio.h>

std::string 
CmdlineExecutor::Popen( const std::string& cmd )
{
    return GetImpl()->popen(cmd);
}

std::string 
CmdlineExecutor::popen( const std::string& cmd )
{ 
	FILE* pipe = ::popen(cmd.c_str(), "r");
	if (!pipe)
		return "ERROR";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}