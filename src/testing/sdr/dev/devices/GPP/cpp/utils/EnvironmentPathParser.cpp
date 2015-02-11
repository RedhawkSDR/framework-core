#include "EnvironmentPathParser.h"

#include <boost/algorithm/string.hpp>

/**
 * STL algorithm predicate for detecting empty string.
 */
struct IsEmptyString
{
    bool operator()( const std::string& str ) const
    {
        return str.empty();
    }
};

EnvironmentPathParser::EnvironmentPathParser( const std::string& path )
{
    from_string(path);
}

EnvironmentPathParser::EnvironmentPathParser( const char* path )
{
    if( path )
    {
        from_string(path);
    }
}

void
EnvironmentPathParser::merge_front( const std::string& path )
{
    if( std::find(paths.begin(), paths.end(), path) == paths.end() )
    {
        paths.insert( paths.begin(), path );
    }
}

void
EnvironmentPathParser::from_string( const std::string& path )
{
    boost::split( paths, path, boost::is_any_of(std::string(":")), boost::algorithm::token_compress_on );
    strip_empty_paths();
}

void 
EnvironmentPathParser::strip_empty_paths()
{
    paths.erase( std::remove_if(paths.begin(), paths.end(), IsEmptyString()), paths.end() );
}

std::string
EnvironmentPathParser::to_string() const
{
    return boost::join( paths, ":" );
}
