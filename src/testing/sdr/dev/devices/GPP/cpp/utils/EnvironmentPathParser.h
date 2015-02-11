#ifndef ENVIRONMENT_PATH_PARSER_H_
#define ENVIRONMENT_PATH_PARSER_H_

#include <string>
#include <vector>

/**
 * EnvironmentPathParser provides operations to read, write, and modify
 * environment path strings (e.g. LD_LIBRARY_PATH).
 */
class EnvironmentPathParser
{
public:
    /**
     * Constructor from std::string.  Extracts paths from path.
     * @param path Path string in the format PATH1:PATH2:PATH3
     */
    EnvironmentPathParser( const std::string& path="" );
    
    /**
     * Constructor from const char*.  Supports NULL pointers, as returned from
     * getenv() on unknown environment variable.
     * @param path Path string in the format PATH1:PATH2:PATH3
     */
    EnvironmentPathParser( const char* path );

    void from_string( const std::string& path );
    std::string to_string() const;
    
    void merge_front( const std::string& path );
    
private:
    void strip_empty_paths();
    
private:
    std::vector<std::string> paths;
};

#endif
