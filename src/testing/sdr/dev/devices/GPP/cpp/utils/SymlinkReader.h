#ifndef SYMLINK_READER_H_
#define SYMLINK_READER_H_

#include "OverridableSingleton.h"

#include <string>

class SymlinkReader : public OverridableSingleton<SymlinkReader>
{
public:
    virtual ~SymlinkReader(){}
    static std::string ReadLink(const std::string& filename);

protected:
    virtual std::string readlink(const std::string& filename);
};

#endif