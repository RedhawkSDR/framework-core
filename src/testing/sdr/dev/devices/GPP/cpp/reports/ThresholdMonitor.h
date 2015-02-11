#ifndef THRESHOLD_MONITOR_H_
#define THRESHOLD_MONITOR_H_

#include "Reporting.h"
#include "../utils/EventDispatcher.h"
#include "../struct_props.h"

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <functional>
#include <sstream>

class ThresholdMonitor :  public Reporting, public EventDispatcherMixin<threshold_event_struct>
{
public:
    ThresholdMonitor( const std::string& message_class, const std::string& resource_id ):
    resource_id_(resource_id),
    message_class_(message_class)
    {}

    ThresholdMonitor( const std::string& source_id, const std::string& resource_id, const std::string& message_class ):
    source_id_(source_id),
    resource_id_(resource_id),
    message_class_(message_class)
    {}

    virtual void update() = 0;
    void report(){ update(); }

    virtual std::string get_threshold() const = 0;
    virtual std::string get_measured() const = 0;

    virtual bool is_threshold_exceeded() const = 0;

    std::string get_source_id() const{ return source_id_; }
    std::string get_resource_id() const{ return resource_id_; }
    std::string get_message_class() const{ return message_class_; }

protected:
    void dispatch_message() const
    {
        threshold_event_struct message;
        message.source_id = get_source_id();
        message.resource_id = get_resource_id();
        message.threshold_class = get_message_class();
        message.type = get_message_type();
        message.threshold_value = get_threshold();
        message.measured_value = get_measured();
        message.message = get_message_string();
        message.timestamp = time(NULL);

        dispatch(message);
    }
    std::string get_message_type() const
    {
        return is_threshold_exceeded() ? "THRESHOLD_EXCEEDED" : "THRESHOLD_NOT_EXCEEDED";
    }
    std::string get_message_string() const
    {
        std::stringstream sstr;
        std::string exceeded_or_not( is_threshold_exceeded() ? "" : "not " );

        sstr << get_message_class() << " threshold " << exceeded_or_not << "exceeded "
             << "(resource_id=" << get_resource_id()
             << " threshold_value=" << get_threshold()
             << " measured_value=" << get_measured() << ")";

        return sstr.str();
    }

private:
    const std::string source_id_;
    const std::string resource_id_;
    const std::string message_class_;
};

template<class DATA_TYPE, class COMPARISON_FUNCTION = std::less<DATA_TYPE> >
class GenericThresholdMonitor : public ThresholdMonitor
{
public:
    typedef DATA_TYPE DataType;
    typedef boost::function< DataType() > QueryFunction;

public:
    GenericThresholdMonitor( const std::string& message_class, const std::string& resource_id, QueryFunction threshold, QueryFunction measured ):
    ThresholdMonitor(message_class, resource_id),
    threshold_(threshold),
    measured_(measured),
    threshold_value_( threshold() ),
    measured_value_( measured() ),
    prev_threshold_exceeded_(false)
    {
    }

    GenericThresholdMonitor( const std::string& source_id, const std::string& resource_id, const std::string& message_class, QueryFunction threshold, QueryFunction measured ):
    ThresholdMonitor(source_id, resource_id, message_class),
    threshold_(threshold),
    measured_(measured),
    threshold_value_( threshold() ),
    measured_value_( measured() ),
    prev_threshold_exceeded_(false)
    {
    }

    void update()
    {
        threshold_value_ = threshold_();
        measured_value_ = measured_();

        if( prev_threshold_exceeded_ != is_threshold_exceeded() )
        {
            dispatch_message();
        }

        prev_threshold_exceeded_ = is_threshold_exceeded();
    }

    std::string get_threshold() const{ return boost::lexical_cast<std::string>(threshold_value_); }
    std::string get_measured() const{ return boost::lexical_cast<std::string>(measured_value_); }

    bool is_threshold_exceeded() const
    {
        return COMPARISON_FUNCTION()( get_measured_value(), get_threshold_value() );
    }

    DataType get_threshold_value() const { return threshold_value_; }
    DataType get_measured_value() const { return measured_value_; }

private:
    QueryFunction threshold_;
    QueryFunction measured_;

    DataType threshold_value_;
    DataType measured_value_;
    bool prev_threshold_exceeded_;
};



#endif
