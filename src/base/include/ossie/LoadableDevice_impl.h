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

#ifndef LOADABLE_DEVICE_IMPL_H
#define LOADABLE_DEVICE_IMPL_H

#include <vector>
#include <map>
#include "Resource_impl.h"
#include "Device_impl.h"
#include "CF/cf.h"
#include <boost/filesystem/path.hpp>

typedef std::multimap<std::string, std::string, std::less<std::string>, std::allocator<std::pair<std::string, std::string> > >
copiedFiles_type;

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

class sharedLibraryStorage {
public:
    void setFilename(std::string _filename) {
        filename = _filename;
    }
    void addModification(const std::string _path_to_modify, const std::string &_path_modification) {
        modifications.push_back(std::make_pair(_path_to_modify, _path_modification));
    };
    std::string filename;
    std::vector<std::pair<std::string,std::string> > modifications;
};

class envStateContainer {
public:
    envStateContainer() {
        if (getenv("LD_LIBRARY_PATH"))
            ld_lib_path = getenv("LD_LIBRARY_PATH");
        else
            ld_lib_path.clear();
        if (getenv("PYTHONPATH"))
            pythonpath = getenv("PYTHONPATH");
        else
            pythonpath.clear();
        if (getenv("CLASSPATH"))
            classpath = getenv("CLASSPATH");
        else
            classpath.clear();
        if (getenv("OCTAVE_PATH"))
            octave_path = getenv("OCTAVE_PATH");
        else
            octave_path.clear();
    };
    
    void set() {
        setenv("LD_LIBRARY_PATH", ld_lib_path.c_str(), 1);
        setenv("PYTHONPATH", pythonpath.c_str(), 1);
        setenv("CLASSPATH", classpath.c_str(), 1);
        setenv("OCTAVE_PATH", octave_path.c_str(), 1);
    };

    std::string ld_lib_path, pythonpath, classpath, octave_path;
};

class envState {
public:
    envState() {
        if (getenv("LD_LIBRARY_PATH"))
            ld_lib_path = getenv("LD_LIBRARY_PATH");
        else
            ld_lib_path.clear();
        if (getenv("PYTHONPATH"))
            pythonpath = getenv("PYTHONPATH");
        else
            pythonpath.clear();
        if (getenv("CLASSPATH"))
            classpath = getenv("CLASSPATH");
        else
            classpath.clear();
        if (getenv("OCTAVE_PATH"))
            octave_path = getenv("OCTAVE_PATH");
        else
            octave_path.clear();
    };
    ~envState() {
        setenv("LD_LIBRARY_PATH", ld_lib_path.c_str(), 1);
        setenv("PYTHONPATH", pythonpath.c_str(), 1);
        setenv("CLASSPATH", classpath.c_str(), 1);
        setenv("OCTAVE_PATH", octave_path.c_str(), 1);
    };
private:
    std::string ld_lib_path, pythonpath, classpath, octave_path;
};

/* CLASS DEFINITION *******************************************************************************
 ************************************************************************************************ */
class LoadableDevice_impl: public virtual POA_CF::LoadableDevice,
    public  Device_impl
{
    ENABLE_LOGGING

public:
    LoadableDevice_impl (char*, char*, char*, char*);
    LoadableDevice_impl (char*, char*, char*, char*, CF::Properties capacities);
    LoadableDevice_impl (char*, char*, char*, char*, char*);
    LoadableDevice_impl (char*, char*, char*, char*, CF::Properties capacities, char*);
    virtual ~LoadableDevice_impl ();

    void  load (CF::FileSystem_ptr fs, const char* fileName,
          CF::LoadableDevice::LoadType loadKind)
    throw (CF::LoadableDevice::LoadFail, CF::InvalidFileName,
           CF::LoadableDevice::InvalidLoadKind, CF::Device::InvalidState,
           CORBA::SystemException);
    void do_load (CF::FileSystem_ptr fs, const char* fileName, CF::LoadableDevice::LoadType loadKind)
    throw (CF::LoadableDevice::LoadFail, CF::InvalidFileName,
           CF::LoadableDevice::InvalidLoadKind, CF::Device::InvalidState,
           CORBA::SystemException);

    void  unload (const char* fileName)
    throw (CF::InvalidFileName, CF::Device::InvalidState,
           CORBA::SystemException);
    void do_unload (const char* fileName) 
    throw (CF::InvalidFileName, CF::Device::InvalidState,
           CORBA::SystemException);

    bool isFileLoaded (const char* fileName);

    void configure (const CF::Properties& configProperties)
    throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

    void setTransferSize( uint64_t  xfersize ){
      if ( xfersize > 0 ) transferSize=xfersize;
    }


protected:

    void incrementFile (std::string);
    void decrementFile (std::string);
    std::map<std::string, int> loadedFiles;
    std::map<std::string, CF::FileSystem::FileType> fileTypeTable;
    copiedFiles_type copiedFiles;
    boost::recursive_mutex load_execute_lock;
    void update_ld_library_path (CF::FileSystem_ptr fs, const char* fileName, CF::LoadableDevice::LoadType loadKind) throw (CORBA::SystemException, CF::Device::InvalidState, CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName, CF::LoadableDevice::LoadFail);
    void update_octave_path (CF::FileSystem_ptr fs, const char* fileName, CF::LoadableDevice::LoadType loadKind) throw (CORBA::SystemException, CF::Device::InvalidState, CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName, CF::LoadableDevice::LoadFail);
    void merge_front_environment_path( const char* environment_variable, const std::string& path ) const;
    std::map<std::string, sharedLibraryStorage> sharedPkgs;
    envStateContainer initialState;
    void update_path(sharedLibraryStorage &packageDescription);
    void update_selected_paths(std::vector<sharedLibraryStorage> &paths);

    CORBA::LongLong           transferSize;          // block transfer size when loading files

 private:
    LoadableDevice_impl(); // No default constructor
    LoadableDevice_impl(LoadableDevice_impl&); // No copying

    void _loadTree(CF::FileSystem_ptr fs, std::string remotePath, boost::filesystem::path& localPath, std::string fileKey);
    void _deleteTree(const std::string &fileKey);
    bool _treeIntact(const std::string &fileKey);
    void _copyFile(CF::FileSystem_ptr fs, const std::string &remotePath, const std::string &localPath, const std::string &fileKey);

    void _init();


};

#endif

