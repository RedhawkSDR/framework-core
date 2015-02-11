#include "NicInterfaceFilter.h"

#include <boost/regex.hpp>
#include <string>
#include <iostream>

NicInterfaceFilterRegex::NicInterfaceFilterRegex( const NicInterfaceRegexes& nic_interface_regexes ):
nic_interface_regexes_(nic_interface_regexes),
regex_(new boost::regex())
{
    compile_regex();
}

bool
NicInterfaceFilterRegex::match( const std::string& nic )
{
    if( is_regex_out_of_date() )
        compile_regex();
    
    return boost::regex_match( nic, *regex_ );
}

bool
NicInterfaceFilterRegex::is_regex_out_of_date() const
{
    return nic_interface_regexes_ != cached_nic_interface_regexes_;
}

void
NicInterfaceFilterRegex::compile_regex()
{
    std::string regex_str;
    
    NicInterfaceRegexes::const_iterator i = nic_interface_regexes_.begin();
    while( i!=nic_interface_regexes_.end() )
    {
        regex_str += *i;
        ++i;
        if( i!=nic_interface_regexes_.end() )
            regex_str += "|";
    }
    
    regex_->assign(regex_str);
    
    cached_nic_interface_regexes_ = nic_interface_regexes_;
}


NicInterfaceFilter::NicInterfaceFilter( const NicInterfaceRegexes& nic_interface_regexes, 
                                        const NicStates& nic_states,
                                        NicStates& filtered_nic_states ):
filter_regex_(nic_interface_regexes),
nic_states_(nic_states),
filtered_nic_states_(filtered_nic_states)
{
    filter();
}

void 
NicInterfaceFilter::filter()
{
    filtered_nic_states_.clear();
    for( NicStates::const_iterator i=nic_states_.begin(); i!=nic_states_.end(); ++i )
    {
        if( filter_regex_.match(i->first) )
            filtered_nic_states_.insert( std::make_pair(i->first, i->second) );
    }
}
