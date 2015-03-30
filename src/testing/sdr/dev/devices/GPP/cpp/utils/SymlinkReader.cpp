#include "SymlinkReader.h"
#include "IOError.h"

#include <sstream>

#include <string.h>
#include <errno.h>
#include <unistd.h>

std::string
SymlinkReader::ReadLink(const std::string& filename)
{
    return GetImpl()->readlink(filename);
}

std::string
SymlinkReader::readlink(const std::string& filename)
{
	char buffer[1024];
	int path_size;

	if( (path_size = ::readlink(filename.c_str(), buffer, sizeof(buffer))) == -1 )
	{
		std::stringstream errstr;
		errstr << "Error reading symlink (filename=" << filename << " msg=" << strerror(errno) << ")";
		throw IOError(errstr.str());
	}

	return std::string(buffer, path_size);
}
