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
#ifndef LOG_HELPERS_H
#define LOG_HELPERS_H

#include <string>
#include <vector>
#include <exception>
#include <boost/shared_ptr.hpp>
#include <ossie/CF/LogInterfaces.h>
/**
The logging namespace contain common routines for configuration
and manipulation of the logging interface.
*/

namespace ossie
{

  namespace logging {
    //
    // This keys of the table are tokens to perform a match search against a stream of data. The values for the
    // associative array are used to replace the contents of the matched tokens.
    //
    // The search tokens and replacements will be use to create a regular expression table that is feed
    // to the boost::regex_replace method.
    //
    typedef std::map<  std::string, std::string >       MacroTable;
  
    typedef enum { JAVA_PROPS=0, XML_PROPS }            LogConfigFormatType;


    struct ResourceCtx {
      std::string name;
      std::string instance_id;
      std::string domain_name;
      std::string dom_path;
      ResourceCtx( const std::string &name,
		    const std::string &id,
		    const std::string &dpath );
      
      virtual ~ResourceCtx() {};
      virtual void apply( MacroTable &tbl );

      std::string  get_path( ) { return dom_path; };

    };
    struct ComponentCtx : public ResourceCtx {
      std::string waveform;
      std::string waveform_id;

      ComponentCtx( const std::string &name,
		    const std::string &id,
		    const std::string &dpath );
      ~ComponentCtx() {};
      virtual void apply( MacroTable &tbl );
    };

    struct ServiceCtx : public ResourceCtx {
      std::string device_mgr;
      std::string device_mgr_id;
      ServiceCtx( const std::string &name,
		  const std::string &dpath );
      ~ServiceCtx() {};
      virtual void apply( MacroTable &tbl );
    };

    struct DeviceCtx : public ResourceCtx {
      std::string device_mgr;
      std::string device_mgr_id;
      DeviceCtx( const std::string &name,
		 const std::string &id,
		  const std::string &dpath );
      ~DeviceCtx() {};
      virtual void apply( MacroTable &tbl );
    };

    struct DeviceMgrCtx : public ResourceCtx {
      DeviceMgrCtx( const std::string &name,
		    const std::string &id,
		    const std::string &dpath );
      ~DeviceMgrCtx() {};
      virtual void apply( MacroTable &tbl );
    };


    //
    // Resource Context information used to resolve macros in configuration files
    //
    typedef boost::shared_ptr< ossie::logging::ResourceCtx >    ResourceCtxPtr;
      
    //
    // GetDefaultMacros
    //
    // returns a default set of search tokens and replacments strings...
    //
    MacroTable GetDefaultMacros();

    //
    // ExpandMacros
    //
    // Process contents of src against of set of macro definitions contained in ctx.
    // The contents of ctx will be used to generate a set of regular expressions that can
    // search src for tokens and substitute their contents.
    //
    // @return string object containing any subsitutions
    //
    std::string  ExpandMacros ( const std::string &src, const MacroTable &ctx );

    //
    // ResolveHostInfo
    //
    // Resolve host and ip address where this resource is running
    //
    void ResolveHostInfo( MacroTable &tbl );

    //
    // SetResourceInfo
    //
    // Set the resource context
    //
    void SetResourceInfo( MacroTable &tbl, const ResourceCtx &ctx );

    //
    // SetComponentInfo
    //
    // Set the component context
    //
    void SetComponentInfo( MacroTable &tbl, const ComponentCtx &ctx );

    //
    // SetServiceInfo
    //
    // Set the service context
    //
    void SetServiceInfo( MacroTable &tbl, const ServiceCtx &ctx );

    //
    // SetDeviceInfo
    //
    // Set the device context
    //
    void SetDeviceInfo( MacroTable &tbl, const DeviceCtx &ctx );

    //
    // SetDeviceMgrInfo
    //
    // Set the device manager context
    //
    void SetDeviceMgrInfo( MacroTable &tbl, const DeviceMgrCtx &ctx );


    //
    // ConvertLogLevel
    //
    // Convert from command line argument debug level to CF::Logging
    //
    CF::LogLevel ConvertLogLevel( int oldstyle_level );



    void SetLevel( const std::string  &logid, int newLevel );


    void SetLogLevel( const std::string  &logid, CF::LogLevel newLevel );

    //
    // GetDefaultConfig
    //
    // return default logging configuration information
    //
    std::string  GetDefaultConfig();

    // 
    // Get Logging Configuration file from an SCA file system and
    // copy to local file system
    //
    std::string  CacheSCAFile( const std::string &sca_url );

    // 
    // Get Logging Configuration file from an SCA file system and
    // return the contents as a string
    //
    // @param  sca_url : format as follows sca://?fs=<file system IOR>/path/to/file
    //
    std::string  GetSCAFileContents( const std::string &sca_url) throw ( std::exception );

    //
    // Resolve the location of the URL and return its contents as a string.
    //
    //
    //
    std::string  GetConfigFileContents( const std::string &url ) throw ( std::exception );


    // 
    //
    // Set the default logging configuration to be standard out and msg level == Info
    //
    void ConfigureDefault();

    //
    // 1.9 and prior logging configuration method,
    //
    // Set the default logging configuration to be standard out and msg level == Info
    //
    void Configure(const char* logcfgUri="", int logLevel=3 );

    //
    // Used by a resource to configure the logging provider 
    //
    //
    void Configure(const std::string &logcfgUri, int logLevel, ResourceCtxPtr ctx);

    //
    // Configure
    //
    // Apply the macro defintion table against the contents of log4j 
    // configuration file (xml or java props) and set the appropriate 
    // log4cxx configuration.
    //
    // @param configure_data contents of log4j properties or xml file
    // @param MacroTable set of tokens to match and substitution values
    // @param cfgContents  returns converted form of configure_data if macro definitions were applied
    //
    void Configure( const std::string  &configure_data, const MacroTable &ctx, std::string &cfgContents );   



  };  // end logging interface

};  // end ossie namespace
#endif
