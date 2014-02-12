/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <iostream>
#include <log4cxx/level.h>
#include "ossie/Logging_impl.h"
#include "ossie/logging/loghelpers.h"

struct null_deleter
{
  void operator()(void const *) const
  {
  }
};


Logging_impl::Logging_impl() :
  _logName(""),
  _logLevel(CF::LogLevels::INFO),
  _logCfgContents(),
  _logCfgURL(""),
  _loggingCtx()
{

  // get default set of macros to fill in
  _loggingMacros = ossie::logging::GetDefaultMacros();

  ossie::logging::ResolveHostInfo( _loggingMacros );

  _logCfgContents = ossie::logging::GetDefaultConfig();

  // setup logger to be root by default and assign logging level INFO`
  getLogger( _logName, true );
};



void  Logging_impl::setLoggingMacros ( ossie::logging::MacroTable &tbl, bool applyCtx ) {

  // copy over contents
  _loggingMacros = tbl;

  if ( applyCtx  ){
    if ( _loggingCtx ) {
      _loggingCtx->apply( _loggingMacros );
    }
  }
}

void Logging_impl::setResourceContext( ossie::logging::ResourceCtxPtr ctx ) {
  if ( ctx ) {
    ctx->apply( _loggingMacros );
    _loggingCtx = ctx;
  }
}


void Logging_impl::setLoggingContext( ossie::logging::ResourceCtxPtr ctx ) {

  std::cout << "Logging_impl setLoggingContext START:" << std::endl; 
  if ( ctx ) {
    std::cout << "Logging_impl setLoggingContext Apply Macro Context:" << std::endl; 
    ctx->apply( _loggingMacros );
    _loggingCtx = ctx;
  }
  else if ( _loggingCtx ) {
    _loggingCtx->apply( _loggingMacros );
  }

  std::cout << "Logging_impl setLoggingContext setLogConfigURL:" << std::endl; 
  setLogConfigURL( _logCfgURL.c_str() );

  std::cout << "Logging_impl setLoggingContext setLogLevel:" << std::endl; 
  setLogLevel( _logName.c_str(), _logLevel );

  std::cout << "Logging_impl setLoggingContext END" << std::endl; 

}


void Logging_impl::setLoggingContext( const std::string &logcfg_url, int logLevel, ossie::logging::ResourceCtxPtr ctx ) {

  if ( logcfg_url == "" ) {
    ossie::logging::ConfigureDefault();
  }
  else {
    std::cout << "Logging_impl setLoggingContext START:" << std::endl; 
    if ( ctx ) {
      std::cout << "Logging_impl setLoggingContext Apply Macro Context:" << std::endl; 
      ctx->apply( _loggingMacros );
      _loggingCtx = ctx;
    }

    std::cout << "Logging_impl setLoggingContext setLogConfigURL:" << std::endl; 
    setLogConfigURL( logcfg_url.c_str() );

  }

  std::cout << "Logging_impl setLoggingContext setLogLevel:" << std::endl; 
  setLogLevel( "", ossie::logging::ConvertLogLevel(logLevel) );

  std::cout << "Logging_impl setLoggingContext END" << std::endl; 
}


Logging_impl::LOGGER Logging_impl::getLogger () {
  return _logger;
}

Logging_impl::LOGGER Logging_impl::getLogger (const std::string &logger_name, bool assignToResource) {
  LOGGER retval;
  if ( logger_name == "" ) {
    retval = log4cxx::Logger::getRootLogger();
  }
  else {
    retval=log4cxx::Logger::getLogger(logger_name);
  }
  
  if ( assignToResource ) {
    _logName = logger_name;
    _logger = retval;
    // assign level to new logger
    log_level( _logLevel );
  }
  
  return retval;
}


char *Logging_impl::getLogConfig () {
  return CORBA::string_dup( _logCfgContents.c_str() );
}

void Logging_impl::setLogConfig( const char *config_contents ) {

  if ( logConfigCallback) {
    (*logConfigCallback)( config_contents );
    _logCfgContents = config_contents;
  }
  else {
    ossie::logging::Configure( config_contents, _loggingMacros, _logCfgContents );
  }
  
}

void Logging_impl::setLogConfigURL( const char *in_url ) {

  //
  // Get File contents....
  //
  std::string url("");
  try{
    if ( in_url ) url=in_url;

    std::string config_contents = ossie::logging::GetConfigFileContents(url);

    if ( config_contents.size() > 0  ){
      setLogConfig( config_contents.c_str() );
    }
    else {
      LOG4CXX_WARN(_logger, "URL contents could not be resolved, url: " << url );
    }

  }
  catch( std::exception &e ){
      LOG4CXX_WARN(_logger, "Exception caught during logging configuration using URL, url: " << url );
  }

}


void Logging_impl::setLogLevel( const char *logger_id, const CF::LogLevel newLevel ) 
  throw (CF::UnknownIdentifier)
{
  if ( logLevelCallback ) {
    (*logLevelCallback)(logger_id, newLevel);
  }
  else {
    std::string logid("");
    if ( logger_id )  logid=logger_id;
    ossie::logging::SetLevel( logid, newLevel );
  }
}

CF::LogLevel Logging_impl::log_level() {
  return _logLevel;
}


void Logging_impl::log_level( const CF::LogLevel newLevel ) {

  if ( logLevelCallback ) {
    _logLevel = newLevel;
    (*logLevelCallback)( "", newLevel);
  }
  else {
    _logLevel = newLevel;
    log4cxx::LevelPtr level;
    if ( newLevel == CF::LogLevels::OFF ) level=log4cxx::Level::getOff();
    if ( newLevel == CF::LogLevels::FATAL ) level=log4cxx::Level::getFatal();
    if ( newLevel == CF::LogLevels::ERROR ) level=log4cxx::Level::getError();
    if ( newLevel == CF::LogLevels::WARN ) level=log4cxx::Level::getWarn();
    if ( newLevel == CF::LogLevels::INFO) level=log4cxx::Level::getInfo();
    if ( newLevel == CF::LogLevels::DEBUG) level=log4cxx::Level::getDebug();
    if ( newLevel == CF::LogLevels::TRACE) level=log4cxx::Level::getTrace();
    if ( newLevel == CF::LogLevels::ALL) level=log4cxx::Level::getAll();

    // apply new level to resource logger
    if ( _logger ) {
      _logger->setLevel( level );
    }
  }   
}



CF::LogEventSequence *Logging_impl::retrieve_records( CORBA::ULong &howMany,
					  CORBA::ULong startingRecord ) {
  howMany=0;
  CF::LogEventSequence_var seq;
  return  seq._retn();
};

CF::LogEventSequence *Logging_impl::retrieve_records_by_date( CORBA::ULong &howMany,
							      CORBA::ULongLong to_timeStamp ) {
  howMany=0;
  CF::LogEventSequence_var seq;
  return  seq._retn();
};

CF::LogEventSequence *Logging_impl::retrieve_records_from_date( CORBA::ULong &howMany,
								CORBA::ULongLong from_timeStamp ) {
  howMany=0;
  CF::LogEventSequence_var seq;
  return  seq._retn();
};



void Logging_impl::setLogConfigCallback(LogConfigListener *func) {
  logConfigCallback =  boost::shared_ptr< LogConfigListener >(func, null_deleter());
}

void Logging_impl::setLogConfigCallback(LogConfigCallbackFn func) {
  logConfigCallback =  boost::make_shared< StaticLogConfigListener >(func );
}

void Logging_impl::setLogLevelCallback(LogLevelListener *func) {
  logLevelCallback =  boost::shared_ptr< LogLevelListener >(func, null_deleter());
}

void Logging_impl::setLogLevelCallback(LogLevelCallbackFn func) {
  logLevelCallback =  boost::make_shared< StaticLogLevelListener >(func);
}



