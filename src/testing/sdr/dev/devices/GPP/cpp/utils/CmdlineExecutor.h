#ifndef CMDLINE_EXECUTOR_H_
#define CMDLINE_EXECUTOR_H_

#include "OverridableSingleton.h"

#include <string>

class CmdlineExecutor : public OverridableSingleton<CmdlineExecutor>
{
public:
    virtual ~CmdlineExecutor(){}

    static std::string Popen( const std::string& cmd );
    
protected:
    virtual std::string popen( const std::string& cmd );
};

#endif