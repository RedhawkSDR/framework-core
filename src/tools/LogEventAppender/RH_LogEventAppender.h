#ifndef RH_LogEvent_APPENDER_H
#define RH_LogEvent_APPENDER_H
#include <string>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/logstring.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/pool.h>
#include <boost/shared_ptr.hpp>
#include "corba.h"
 
namespace log4cxx
{
	 
class RH_LogEventAppender : public AppenderSkeleton
{
 public:
  DECLARE_LOG4CXX_OBJECT(RH_LogEventAppender)
 
    BEGIN_LOG4CXX_CAST_MAP()
    LOG4CXX_CAST_ENTRY(RH_LogEventAppender)
    LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
    END_LOG4CXX_CAST_MAP()

  
  RH_LogEventAppender();
  virtual ~RH_LogEventAppender();

  //
  // Called by log4cxx internals to process options
  //
  void setOption(const LogString& option, const LogString& value);

  void activateOptions( log4cxx::helpers::Pool& p);

  // This method is called by the AppenderSkeleton#doAppend method
  void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);

  void close();
 
  bool isClosed() const { return closed; }
	 
  bool requiresLayout() const { return true; }

 private:

  typedef boost::shared_ptr< corba::PushEventSupplier >     PushEventSupplierPtr;

  std::vector< std::string >                                ArgList;

  //
  // perform connect operation to establish a corba context 
  //
  int                                      connect_();
  
  //
  // Command line arguments used to configure corba util methods
  //
  LogString                                 _args;

  //
  // handle to corba context 
  //
  corba::OrbPtr                             _orb;

  //
  // channel name
  //
  LogString                                 channelName;

  //
  // naming context 
  //
  LogString                                 nameContext;

  //
  // Producer Identifier
  //
  LogString                                 prodId;

  //
  // Producer Name
  //
  LogString                                 prodName;

  //
  // Producer FQN - fully qualified domain name for resource
  //
  LogString                                 prodFQN;

  //
  // channel name, shadow variable
  //
  LogString                                 _channelName;

  //
  // naming context, shadow variable
  //
  LogString                                 _nameContext;

  //
  // Handle to requested channel, might want to make this a vector... and this class a singleton
  //
  PushEventSupplierPtr                      _event_channel;

  // number of times to retry before calling it quits.. reset after each successfull connection ( -1 try forever )
  int                                       _reconnect_retries;

  // number of milliseconds to delay before retrying to connect to CORBA resoure
  int                                       _reconnect_delay;

  //  prevent copy and assignment statements
  RH_LogEventAppender(const RH_LogEventAppender&);
  RH_LogEventAppender& operator=(const RH_LogEventAppender&);

  };
	 
}; // end of namespace
#endif
