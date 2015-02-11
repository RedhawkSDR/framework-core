#ifndef IOERROR_H_
#define IOERROR_H_

#include <string>
#include <stdexcept>

class IOError : public std::runtime_error
{
public:
    explicit IOError(const std::string& arg): std::runtime_error(arg){};
};

#endif