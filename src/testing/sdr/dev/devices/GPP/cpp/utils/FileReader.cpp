#include "FileReader.h"
#include "IOError.h"

#include <sstream>
#include <fstream>

std::string 
FileReader::ReadFile( const std::string& filename )
{
    return GetImpl()->read_file(filename);
}

std::string
FileReader::read_file( const std::string& filename )
{
    std::ifstream fstr(filename.c_str());
    
    if( !fstr.good() )
    {
        std::stringstream errstr;
        errstr << "Error opening file for reading (" << filename << ")";
        throw IOError( errstr.str() );
    }
  
    try
    {
        return std::string((std::istreambuf_iterator<char>(fstr)), std::istreambuf_iterator<char>());
    }
    catch( const std::exception& e )
    {
        std::stringstream errstr;
        errstr << "Error reading file (" << filename << " msg=\"" << e.what() << "\")";
        throw IOError( errstr.str() );
    }
}

