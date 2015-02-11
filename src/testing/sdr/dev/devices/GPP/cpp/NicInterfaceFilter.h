#ifndef NIC_INTERFACE_FILTER_H_
#define NIC_INTERFACE_FILTER_H_

#include <boost/regex_fwd.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <map>

typedef std::vector<std::string> NicInterfaceRegexes;

class NicInterfaceFilterRegex
{
public:
    NicInterfaceFilterRegex( const NicInterfaceRegexes& nic_interface_regexes );
    
    bool match( const std::string& nic );
    
private:
    bool is_regex_out_of_date() const;
    void compile_regex();
    
private:
    const NicInterfaceRegexes& nic_interface_regexes_;
    NicInterfaceRegexes cached_nic_interface_regexes_;
    boost::shared_ptr<boost::regex> regex_;
};

class NicState;

class NicInterfaceFilter
{
public:
    typedef std::map<std::string, boost::shared_ptr<NicState> > NicStates;
    
public:
    NicInterfaceFilter( const NicInterfaceRegexes& nic_interface_regexes, 
                        const NicStates& nic_states,
                        NicStates& filtered_nic_states );
    
    void filter();
    
private:
    NicInterfaceFilterRegex filter_regex_;
    const NicStates& nic_states_;
    NicStates& filtered_nic_states_;
};

#endif
