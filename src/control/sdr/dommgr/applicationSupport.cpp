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


#include <sstream>
#include <vector>
#include <stdlib.h>
#include <omniORB4/CORBA.h>

#include <ossie/debug.h>
#include <ossie/SoftPkg.h>
#include <ossie/Properties.h>
#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/prop_utils.h>
#include <ossie/CorbaUtils.h>

#include "applicationSupport.h"
#include "PersistenceStore.h"

using namespace ossie;

static void addProperty(const CF::DataType& dt, CF::Properties& prop)
{
    for (unsigned int ii = 0; ii < prop.length(); ++ii) {
        if (strcmp(dt.id, prop[ii].id) == 0) {
            // Overwrite existing value
            prop[ii].value = dt.value;
            return;
        }
    }

    // New id, add property at end
    unsigned int index = prop.length();
    prop.length(index + 1);
    prop[index] = dt;
}

////////////////////////////////////////////////////
/**
 * UsesDeviceInfo member function definitions
 */
UsesDeviceInfo::UsesDeviceInfo(const std::string& _id, const std::string& _type, const std::vector<ossie::SPD::PropertyRef>& _properties) :
    id(_id),
    type(_type),
    properties(_properties)
{

}

UsesDeviceInfo::UsesDeviceInfo(const std::string& _id, const std::string& _type, const std::vector<ossie::SoftwareAssembly::PropertyRef>& _sadDeps) :
        id(_id),
        type(_type),
        sadDeps(_sadDeps)
{

}

UsesDeviceInfo::~UsesDeviceInfo()
{
}

const std::string& UsesDeviceInfo::getId() const
{
    return id;
}

const std::string& UsesDeviceInfo::getType() const
{
    return type;
}

const std::vector<ossie::SPD::PropertyRef>& UsesDeviceInfo::getProperties() const
{
    return properties;
}

const std::vector<ossie::SoftwareAssembly::PropertyRef>& UsesDeviceInfo::getSadDeps() const
{
    return sadDeps;
}

const std::string& UsesDeviceInfo::getAssignedDeviceId() const
{
    return assignedDeviceId;
}

void UsesDeviceInfo::setAssignedDeviceId(const std::string& deviceId)
{
    assignedDeviceId = deviceId;
}

void UsesDeviceInfo::clearAssignedDeviceId()
{
    assignedDeviceId.clear();
}

////////////////////////////////////////////////////
/**
 * UsesDeviceContext member function definitions
 */
PREPARE_LOGGING(UsesDeviceContext);

UsesDeviceContext::UsesDeviceContext() :
    usesDevices()
{
}

UsesDeviceContext::~UsesDeviceContext()
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        delete usesDevices[ii];
    }
    usesDevices.clear();
}

void UsesDeviceContext::addUsesDevice(UsesDeviceInfo* _usesDevice)
{
    usesDevices.push_back(_usesDevice);
}

const std::vector<UsesDeviceInfo*>& UsesDeviceContext::getUsesDevices() const
{
    return usesDevices;
}

const UsesDeviceInfo* UsesDeviceContext::getUsesDeviceById(const std::string& id) const
{
    for (size_t ii = 0; ii < usesDevices.size(); ++ii) {
        if (usesDevices[ii]->getId() == id) {
            return usesDevices[ii];
        }
    }

    return 0;
}


////////////////////////////////////////////////////
/**
 * ImplementationInfo member function definitions
 */
PREPARE_LOGGING(ImplementationInfo);

ImplementationInfo::ImplementationInfo(const SPD::Implementation& spdImpl) :
    id(spdImpl.getID()),
    codeType(),
    localFileName(),
    entryPoint(),
    processorDeps(spdImpl.getProcessors()),
    osDeps(spdImpl.getOsDeps()),
    dependencyProperties()
{
    setLocalFileName(spdImpl.getCodeFile());
    setEntryPoint(spdImpl.getEntryPoint());
    setCodeType(spdImpl.getCodeType());
    setStackSize(spdImpl.code.stacksize.get());
    setPriority(spdImpl.code.priority.get());

    // Create local copies for all of the usesdevice entries for this implementation.
    LOG_TRACE(ImplementationInfo, "Loading component implementation uses device")
    const std::vector<SPD::UsesDevice>& spdUsesDevices = spdImpl.getUsesDevices();
    for (size_t ii = 0; ii < spdUsesDevices.size(); ++ii) {
        const SPD::UsesDevice& spdUsesDev = spdUsesDevices[ii];
        UsesDeviceInfo* usesDevice = new UsesDeviceInfo(spdUsesDev.getID(), spdUsesDev.getType(),
                    spdUsesDev.getDependencies());
        addUsesDevice(usesDevice);
    }

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation property dependencies")
    const std::vector<ossie::SPD::PropertyRef>& dependencies = spdImpl.getDependencies();
    std::vector<ossie::SPD::PropertyRef>::const_iterator ii;
    for (ii = dependencies.begin(); ii != dependencies.end(); ++ii) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation property dependency '" << *ii);
        addDependencyProperty(*ii);
    }
}

ImplementationInfo::~ImplementationInfo()
{
    for (std::vector<SoftpkgInfo*>::iterator ii = softPkgDependencies.begin(); ii != softPkgDependencies.end(); ++ii) {
        delete (*ii);
    }
}

ImplementationInfo* ImplementationInfo::buildImplementationInfo(CF::FileManager_ptr fileMgr, const SPD::Implementation& spdImpl)
{
    ImplementationInfo* impl = new ImplementationInfo(spdImpl);

    // Handle allocation property dependencies
    LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependencies")
    const std::vector<ossie::SPD::SoftPkgRef>& softpkgDependencies = spdImpl.getSoftPkgDependencies();
    std::vector<ossie::SPD::SoftPkgRef>::const_iterator jj;
    for (jj = softpkgDependencies.begin(); jj != softpkgDependencies.end(); ++jj) {
        LOG_TRACE(ImplementationInfo, "Loading component implementation softpkg dependency '" << *jj);
        std::auto_ptr<SoftpkgInfo> softpkg(SoftpkgInfo::buildSoftpkgInfo(fileMgr, jj->localfile.c_str()));
        impl->addSoftPkgDependency(softpkg.release());
    }

    return impl;
}

const std::string& ImplementationInfo::getId() const
{
    return id;
}

const std::vector<std::string>& ImplementationInfo::getProcessorDeps() const
{
    return processorDeps;
}

const std::vector<SoftpkgInfo*>& ImplementationInfo::getSoftPkgDependency() const
{
    return softPkgDependencies;
}

const std::vector<ossie::SPD::NameVersionPair>& ImplementationInfo::getOsDeps() const
{
    return osDeps;
}

CF::LoadableDevice::LoadType ImplementationInfo::getCodeType() const
{
    return codeType;
}

const std::string& ImplementationInfo::getLocalFileName() const
{
    return localFileName;
}

const std::string& ImplementationInfo::getEntryPoint() const
{
    return entryPoint;
}

const CORBA::ULong ImplementationInfo::getStackSize() const
{
    return stackSize;
}

const CORBA::ULong ImplementationInfo::getPriority() const
{
    return priority;
}

const bool ImplementationInfo::hasStackSize() const
{
    return _hasStackSize;
}

const bool ImplementationInfo::hasPriority() const
{
    return _hasPriority;
}

const std::vector<SPD::PropertyRef>& ImplementationInfo::getDependencyProperties() const
{
    return dependencyProperties;
}

void ImplementationInfo::setCodeType(const char* _type)
{
    std::string type(_type);
    if (type == "KernelModule") {
        codeType = CF::LoadableDevice::KERNEL_MODULE;
    } else if (type == "SharedLibrary") {
        codeType = CF::LoadableDevice::SHARED_LIBRARY;
    } else if (type == "Executable") {
        codeType = CF::LoadableDevice::EXECUTABLE;
    } else if (type == "Driver") {
        codeType = CF::LoadableDevice::DRIVER;
    } else {
        LOG_WARN(ImplementationInfo, "Bad code type " << type);
    }
}

void ImplementationInfo::setLocalFileName(const char* fileName)
{
    if (fileName) {
        localFileName = fileName;
    }
}

void ImplementationInfo::setEntryPoint(const char* _entryPoint)
{
    if (_entryPoint) {
        entryPoint = _entryPoint;
    }
}

void ImplementationInfo::setStackSize(const unsigned long long* _stackSize)
{
    _hasStackSize = false;
    if (_stackSize) {
        stackSize = *_stackSize;
        _hasStackSize = true;
    }
}

void ImplementationInfo::setPriority(const unsigned long long* _priority)
{
    _hasPriority = false;
    if (_priority) {
        priority = *_priority;
        _hasPriority = true;
    }
}

void ImplementationInfo::addDependencyProperty(const SPD::PropertyRef& property)
{
    dependencyProperties.push_back(property);
}

void ImplementationInfo::addSoftPkgDependency(SoftpkgInfo* softpkg)
{
    softPkgDependencies.push_back(softpkg);
}

bool ImplementationInfo::checkProcessorAndOs(const Properties& _prf) const
{
    bool matchProcessor = checkProcessor(processorDeps, _prf.getAllocationProperties());
    bool matchOs = checkOs(osDeps, _prf.getAllocationProperties());

    if (!matchProcessor) {
        LOG_DEBUG(ImplementationInfo, "Failed to match component processor to device allocation properties");
    }
    if (!matchOs) {
        LOG_DEBUG(ImplementationInfo, "Failed to match component os to device allocation properties");
    }
    return matchProcessor && matchOs;
}

void ImplementationInfo::clearSelectedDependencyImplementations()
{
    std::vector<ossie::SoftpkgInfo*>::const_iterator iter;
    for (iter = softPkgDependencies.begin(); iter != softPkgDependencies.end(); ++iter) {
        (*iter)->clearSelectedImplementation();
    }
}


PREPARE_LOGGING(SoftpkgInfo);

SoftpkgInfo::SoftpkgInfo(const std::string& spdFileName):
    _spdFileName(spdFileName),
    _selectedImplementation(0)
{
}

SoftpkgInfo::~SoftpkgInfo()
{
    for (ImplementationInfo::List::iterator ii = _implementations.begin(); ii != _implementations.end(); ++ii) {
        delete *ii;
    }
}

const char* SoftpkgInfo::getSpdFileName()
{
    return _spdFileName.c_str();
}

const char* SoftpkgInfo::getName()
{
    return _name.c_str();
}

SoftpkgInfo* SoftpkgInfo::buildSoftpkgInfo(CF::FileManager_ptr fileMgr, const char* spdFileName)
{
    LOG_TRACE(SoftpkgInfo, "Building soft package info from file " << spdFileName);

    std::auto_ptr<ossie::SoftpkgInfo> softpkg(new SoftpkgInfo(spdFileName));

    if (!softpkg->parseProfile(fileMgr)) {
        return 0;
    } else {
        return softpkg.release();
    }
}

bool SoftpkgInfo::parseProfile(CF::FileManager_ptr fileMgr)
{
    try {
        File_stream spd_file(fileMgr, _spdFileName.c_str());
        spd.load(spd_file, _spdFileName.c_str());
        spd_file.close();
    } catch (const ossie::parser_error& e) {
        LOG_ERROR(SoftpkgInfo, "building component info problem; error parsing spd; " << e.what());
        return false;
    } catch (...) {
        LOG_ERROR(SoftpkgInfo, "building component info problem; unknown error parsing spd;");
        return false;
    }

    // Set name from the SPD
    _name = spd.getSoftPkgName();

    // Extract implementation data from SPD file
    const std::vector <SPD::Implementation>& spd_i = spd.getImplementations();

    // Assume only one implementation, use first available result [0]
    for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
        const SPD::Implementation& spdImpl = spd_i[implCount];
        LOG_TRACE(SoftpkgInfo, "Adding implementation " << spdImpl.getID());
        ImplementationInfo* newImpl = ImplementationInfo::buildImplementationInfo(fileMgr, spdImpl);
        addImplementation(newImpl);
    }

    // Create local copies for all of the usesdevice entries for this implementation.
    const std::vector<SPD::UsesDevice>& spdUsesDevices = spd.getUsesDevices();
    for (size_t ii = 0; ii < spdUsesDevices.size(); ++ii) {
        const SPD::UsesDevice& spdUsesDev = spdUsesDevices[ii];
        UsesDeviceInfo* usesDevice = new UsesDeviceInfo(spdUsesDev.getID(), spdUsesDev.getType(),
                                                        spdUsesDev.getDependencies());
        addUsesDevice(usesDevice);
    }

    return true;
}

void SoftpkgInfo::addImplementation(ImplementationInfo* impl)
{
    _implementations.push_back(impl);
}

void SoftpkgInfo::getImplementations(ImplementationInfo::List& res)
{
    std::copy(_implementations.begin(), _implementations.end(), std::back_inserter(res));
}

void SoftpkgInfo::setSelectedImplementation(ImplementationInfo* implementation)
{
    if (std::find(_implementations.begin(), _implementations.end(), implementation) == _implementations.end()) {
        throw std::logic_error("invalid implementation selected");
    }
    _selectedImplementation = implementation;
}

void SoftpkgInfo::clearSelectedImplementation()
{
    if (_selectedImplementation) {
        _selectedImplementation->clearSelectedDependencyImplementations();
        _selectedImplementation = 0;
    }
}

const ImplementationInfo* SoftpkgInfo::getSelectedImplementation() const
{
    return _selectedImplementation;
}

const UsesDeviceInfo* SoftpkgInfo::getUsesDeviceById(const std::string& id) const
{
    const UsesDeviceInfo* uses = UsesDeviceContext::getUsesDeviceById(id);
    if (uses) {
        return uses;
    }

    if (_selectedImplementation) {
        return _selectedImplementation->getUsesDeviceById(id);
    }

    return 0;
}

////////////////////////////////////////////////////
/**
 * ComponentInfo member function definitions
 */
PREPARE_LOGGING(ComponentInfo);

ComponentInfo* ComponentInfo::buildComponentInfoFromSPDFile(CF::FileManager_ptr fileMgr, const char* spdFileName)
{
    LOG_TRACE(ComponentInfo, "Building component info from file " << spdFileName);

    ossie::ComponentInfo* newComponent = new ossie::ComponentInfo(spdFileName);

    if (!newComponent->parseProfile(fileMgr)) {
        delete newComponent;
        return 0;
    }
    
    if (newComponent->spd.getSCDFile() != 0) {
        try {
            File_stream _scd(fileMgr, newComponent->spd.getSCDFile());
            newComponent->scd.load(_scd);
            _scd.close();
        } catch (ossie::parser_error& e) {
            LOG_ERROR(ComponentInfo, "building component info problem; error parsing scd; " << e.what());
            delete newComponent;
            return 0;
        } catch( ... ) {
            LOG_ERROR(ComponentInfo, "building component info problem; unknown error parsing scd;");
            delete newComponent;
            return 0;
        }
    }

    if (newComponent->spd.getPRFFile() != 0) {
        LOG_DEBUG(ComponentInfo, "Loading component properties from " << newComponent->spd.getPRFFile());
        try {
            File_stream _prf(fileMgr, newComponent->spd.getPRFFile());
            LOG_DEBUG(ComponentInfo, "Parsing component properties");
            newComponent->prf.load(_prf);
            LOG_TRACE(ComponentInfo, "Closing PRF file")
            _prf.close();
        } catch (ossie::parser_error& e) {
            LOG_ERROR(ComponentInfo, "building component info problem; error parsing prf; " << e.what());
            delete newComponent;
            return 0;
        } catch( ... ) {
            LOG_ERROR(ComponentInfo, "building component info problem; unknown error parsing prf;");
            delete newComponent;
            return 0;
        }
    }

    if (newComponent->spd.isScaNonCompliant()) {
        newComponent->setIsScaCompliant(false);
    } else {
        newComponent->setIsScaCompliant(true);
    }

    // Extract Properties from the implementation-agnostic PRF file
    // once we match the component to a device we can grab the implementation
    // specific PRF file
    if (newComponent->spd.getPRFFile() != 0) {
        // Handle component properties
        LOG_TRACE(ComponentInfo, "Adding factory params")
        const std::vector<const Property*>& fprop = newComponent->prf.getFactoryParamProperties();
        for (unsigned int i = 0; i < fprop.size(); i++) {
            newComponent->addFactoryParameter(convertPropertyToDataType(fprop[i]));
        }

        LOG_TRACE(ComponentInfo, "Adding exec params")
        const std::vector<const Property*>& eprop = newComponent->prf.getExecParamProperties();
        for (unsigned int i = 0; i < eprop.size(); i++) {
            if (std::string(eprop[i]->getMode()) != "readonly") {
                LOG_TRACE(ComponentInfo, "Adding exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
                newComponent->addExecParameter(convertPropertyToDataType(eprop[i]));
            } else {
                LOG_TRACE(ComponentInfo, "Ignoring readonly exec param " << eprop[i]->getID() << " " << eprop[i]->getName());
            }
        }

        // According to SCA 2.2.2 Appendix D 4.1.1.6 the only properties
        // that are to be sent to allocateCapacity are device
        // properties that are external.  Furthermore, the SPD list's its
        // needs, not in the PRF file, but in the SPD file <dependencies>
        // element
        // prop = prf->getMatchingProperties();
        //for (unsigned int i=0; i < prop->size(); i++) {
        //    newComponent->addAllocationCapacity((*prop)[i]->getDataType());
        //}

        const std::vector<const Property*>& prop = newComponent->prf.getConfigureProperties();
        for (unsigned int i = 0; i < prop.size(); i++) {
            if (!prop[i]->isReadOnly()) {
                LOG_TRACE(ComponentInfo, "Adding configure prop " << prop[i]->getID() << " " << prop[i]->getName() << " " << prop[i]->isReadOnly())
                newComponent->addConfigureProperty(convertPropertyToDataType(prop[i]));
            }
        }
    }

    LOG_TRACE(ComponentInfo, "Done building component info from file " << spdFileName);
    return newComponent;
}

ComponentInfo::ComponentInfo(const std::string& spdFileName) :
    SoftpkgInfo(spdFileName),
    _isAssemblyController(false),
    _isScaCompliant(true),
    assignedDevice()
{
}

ComponentInfo::~ComponentInfo ()
{
}

void ComponentInfo::setIdentifier(const char* _identifier, std::string instance_id)
{
    identifier = _identifier;
    // Per the SCA spec, the identifier is the instantiation ID:waveform_name
    instantiationId = instance_id;
}

void ComponentInfo::setAssignedDevice(boost::shared_ptr<ossie::DeviceNode> device)
{
    assignedDevice = device;
}

void ComponentInfo::setNamingService(const bool _isNamingService)
{
    isNamingService = _isNamingService;
}

void ComponentInfo::setNamingServiceName(const char* _namingServiceName)
{
    namingServiceName = _namingServiceName;
}

void ComponentInfo::setUsageName(const char* _usageName)
{
    if (_usageName != 0) {
        usageName = _usageName;
    }
}

void ComponentInfo::setIsAssemblyController(bool _isAssemblyController)
{
    this->_isAssemblyController = _isAssemblyController;
}

void ComponentInfo::setIsScaCompliant(bool _isScaCompliant)
{
    this->_isScaCompliant = _isScaCompliant;
}

void ComponentInfo::addFactoryParameter(CF::DataType dt)
{
    addProperty(dt, factoryParameters);
}

void ComponentInfo::addExecParameter(CF::DataType dt)
{
    addProperty(dt, execParameters);
}

void ComponentInfo::addConfigureProperty(CF::DataType dt)
{
    addProperty(dt, configureProperties);
}

void ComponentInfo::overrideProperty(const ossie::ComponentProperty* propref) {
    std::string propId = propref->getID();
    LOG_TRACE(ComponentInfo, "Instantiation property id = " << propId)
    const Property* prop = prf.getProperty(propId);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ComponentInfo, "ignoring attempt to override property " << propId << " that does not exist in component")
        return;
    }

    CF::DataType dt = overridePropertyValue(prop, propref);
    overrideProperty(dt.id, dt.value);
}

void ComponentInfo::overrideSimpleProperty(const char* id, const std::string value)
{
    const Property* prop = prf.getProperty(id);
    // Without a prop, we don't know how to convert the strings to the property any type
    if (prop == NULL) {
        LOG_WARN(ComponentInfo, "ignoring attempt to override property " << id << " that does not exist in component")
        return;
    }

    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simple = dynamic_cast<const SimpleProperty*>(prop);
        CORBA::TypeCode_ptr type = ossie::getTypeCode(static_cast<std::string>(simple->getType()));
        CORBA::Any val = ossie::string_to_any(value, type);
        overrideProperty(id, val);
    } else {
        LOG_WARN(ComponentInfo, "attempt to override non-simple property with string value");
    }
}

void ComponentInfo::overrideProperty(const char* id, const CORBA::Any& value)
{
    const Property* prop = prf.getProperty(id);
    if (prop != NULL) {
        if (prop->isReadOnly()) {
            LOG_WARN(ComponentInfo, "ignoring attempt to override readonly property " << id);
            return;
        }
    }
    process_overrides(&configureProperties, id, value);
    process_overrides(&options, id, value);
    process_overrides(&factoryParameters, id, value);
    process_overrides(&execParameters, id, value);
}



void ComponentInfo::process_overrides(CF::Properties* props, const char* id, CORBA::Any value)
{
    LOG_DEBUG(ComponentInfo, "Attempting to override property " << id);
    for (unsigned int i = 0; i < (*props).length(); ++i ) {
        if (strcmp(id, (*props)[i].id) == 0) {
            LOG_DEBUG(ComponentInfo, "Overriding property " << id << " with value " << ossie::any_to_string(value));
            (*props)[i].value = value;
        }
    }
    return;
}

void ComponentInfo::setResourcePtr(CF::Resource_ptr _rsc)
{
    rsc = CF::Resource::_duplicate(_rsc);
}

const char* ComponentInfo::getInstantiationIdentifier()
{
    return instantiationId.c_str();
}

const char* ComponentInfo::getIdentifier()
{
    return identifier.c_str();
}

boost::shared_ptr<ossie::DeviceNode> ComponentInfo::getAssignedDevice()
{
    return assignedDevice;
}

const char* ComponentInfo::getAssignedDeviceId()
{
    if (assignedDevice) {
        return assignedDevice->identifier.c_str();
    } else {
        return "";
    }
}

const bool  ComponentInfo::getNamingService()
{
    return isNamingService;
}

const char* ComponentInfo::getUsageName()
{
    return usageName.c_str();
}

const char* ComponentInfo::getNamingServiceName()
{
    return namingServiceName.c_str();
}

const bool  ComponentInfo::isResource()
{
    return scd.isResource();
}

const bool  ComponentInfo::isConfigurable()
{
    return scd.isConfigurable();
}

const bool  ComponentInfo::isAssemblyController()
{
    return _isAssemblyController;
}

const bool  ComponentInfo::isScaCompliant()
{
    return _isScaCompliant;
}

bool ComponentInfo::isAssignedToDevice() const
{
    return assignedDevice;
}

CF::Properties ComponentInfo::getNonNilConfigureProperties()
{
    return ossie::getNonNilConfigureProperties(configureProperties);
}

CF::Properties ComponentInfo::getConfigureProperties()
{
    return configureProperties;
}

CF::Properties ComponentInfo::getOptions()
{
    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    //  unfortunately this can't happen until an implementation has been chosen
    if (_selectedImplementation) {
        if (_selectedImplementation->hasStackSize()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("STACK_SIZE");  // 3.1.3.3.3.3.6
            options[options.length()-1].value <<= _selectedImplementation->getStackSize();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
        if (_selectedImplementation->hasPriority()) {
            options.length(options.length()+1);
            options[options.length()-1].id = CORBA::string_dup("PRIORITY");  // 3.1.3.3.3.3.7
            options[options.length()-1].value <<= _selectedImplementation->getPriority();  // The specification says it's supposed to be an unsigned long, but the parser is set to unsigned long long
        }
    }

    return options;
}

CF::Properties ComponentInfo::getExecParameters()
{
    return execParameters;
}

CF::Resource_ptr ComponentInfo::getResourcePtr()
{
    return CF::Resource::_duplicate(rsc);
}


////////////////////////////////////////////////////
/**
 * ApplicationInfo member function definitions
 */
PREPARE_LOGGING(ApplicationInfo);

ApplicationInfo::ApplicationInfo()
{
}

ApplicationInfo::~ApplicationInfo()
{
    for (unsigned int ii = 0; ii < usesDevices.size(); ++ii) {
        delete usesDevices[ii];
    }
    usesDevices.clear();
}

const std::vector<SoftwareAssembly::Port>& ApplicationInfo::getExternalPorts() const
{
    return externalPorts;
}

const std::vector<SoftwareAssembly::Property>& ApplicationInfo::getExternalProperties() const
{
    return externalProperties;
}

const CF::Properties ApplicationInfo::getACProperties() const
{
    return acProps;
}

void ApplicationInfo::setACProperties(const CF::Properties& props)
{
    acProps = props;
}

void ApplicationInfo::populateApplicationInfo(const SoftwareAssembly& sad)
{
    // Gets external ports
    const std::vector<SoftwareAssembly::Port>& ports = sad.getExternalPorts();
    for (std::vector<SoftwareAssembly::Port>::const_iterator port = ports.begin(); port != ports.end(); ++port) {
        externalPorts.push_back(*port);
    }

    // Gets external properties
    const std::vector<SoftwareAssembly::Property>& props = sad.getExternalProperties();
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
        externalProperties.push_back(*prop);
    }

    // Gets uses device relationships
    const std::vector<SoftwareAssembly::UsesDevice>& usesDevice = sad.getUsesDevices();
    for (std::vector<SoftwareAssembly::UsesDevice>::const_iterator use = usesDevice.begin(); use != usesDevice.end(); ++use) {
        UsesDeviceInfo* useDev = new UsesDeviceInfo(use->getId(), use->getType(), use->getDependencies());
        addUsesDevice(useDev);
    }
}

void ApplicationInfo::populateExternalProperties(CF::Properties& vals)
{
    for (std::vector<SoftwareAssembly::Property>::iterator prop = externalProperties.begin();
            prop != externalProperties.end();
            ++prop) {
        // Gets the property information
        std::string compId = prop->comprefid;
        std::string propId = prop->propid;
        std::string extId = "";
        if (prop->externalpropid != "") {
            extId = prop->externalpropid;
        } else {
            extId = prop->propid;
        }

        ComponentInfo *comp = findComponentByInstantiationId(prop->comprefid);
        if (comp == 0) {
            LOG_WARN(ApplicationInfo, "Unable to find component: " << prop->comprefid <<
                    " for external property: " << prop->externalpropid);
            continue;
        }

        CF::Properties compProps = comp->getConfigureProperties();
        for (unsigned int i = 0; i < compProps.length(); ++i) {
            if (strcmp(compProps[i].id, propId.c_str()) == 0) {
                int length = vals.length();
                vals.length(length + 1);
                vals[length].id = CORBA::string_dup(extId.c_str());
                vals[length].value = compProps[i].value;
            }
        }
    }
}

void ApplicationInfo::addComponent(ComponentInfo* comp)
{
    components.push_back(comp);
}

ComponentInfo* ApplicationInfo::findComponentByInstantiationId(const std::string id)
{
    for (unsigned int i = 0; i < components.size(); ++i) {
        if (components[i]->getInstantiationIdentifier() == id) {
                return components[i];
        }
    }
    return 0;
}
