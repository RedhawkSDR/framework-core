#ifndef FILE_READER_H_
#define	FILE_READER_H_

#include "OverridableSingleton.h"

#include <string>

class FileReader : public OverridableSingleton<FileReader>
{
public:
    virtual ~FileReader(){}
    static std::string ReadFile( const std::string& filename );
    
protected:
    virtual std::string read_file( const std::string& filename );
};

#endif	

