#ifndef  RH_LOGGER_P_H
#define  RH_LOGGER_P_H

#include <values.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ossie/logging/rh_logger.h>

namespace rh_logger {

  class StdOutLogger : public Logger {

  public:

    static  LoggerPtr  getRootLogger( );
    static  LoggerPtr  getLogger( const std::string &name );
    static  LoggerPtr  getLogger( const char *name );

    virtual ~StdOutLogger() {}
    
    StdOutLogger( const std::string &name );

    StdOutLogger( const char *name );

    void setLevel ( const LevelPtr &newLevel );

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg ) ;

    const LevelPtr&  getEffectiveLevel() const;

  protected:


  private:

    typedef boost::shared_ptr< StdOutLogger > StdOutLoggerPtr;

    static StdOutLoggerPtr   _rootLogger;

    std::ostream   &_os;

  };


#ifdef HAVE_LOG4CXX

  log4cxx::LevelPtr ConvertRHLevelToLog4 ( rh_logger::LevelPtr rh_level );
  rh_logger::LevelPtr ConvertLog4ToRHLevel ( log4cxx::LevelPtr l4_level );

  class L4Logger : public Logger {

  public:

    static  LoggerPtr  getRootLogger( );
    static  LoggerPtr  getLogger( const std::string &name );
    static  LoggerPtr  getLogger( const char *name );

    virtual ~L4Logger() {}
    
    L4Logger( const std::string &name );

    L4Logger( const char *name );

    bool isFatalEnabled() const;
    bool isErrorEnabled() const;
    bool isWarnEnabled() const;
    bool isInfoEnabled() const;
    bool isDebugEnabled() const;
    bool isTraceEnabled() const;

    void setLevel ( const LevelPtr &newLevel );

    LevelPtr getLevel () const;

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg ) ;

    const LevelPtr&  getEffectiveLevel() const;

  private:

    typedef boost::shared_ptr< L4Logger > L4LoggerPtr;

    static L4LoggerPtr   _rootLogger;

    log4cxx::LoggerPtr  l4logger;
  };

#endif

};   // end of rh_logger namespace

#endif
