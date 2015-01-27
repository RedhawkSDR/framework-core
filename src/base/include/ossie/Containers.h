/* 
 * File:   Containers.h
 * Author: pmrobe3
 *
 * Created on December 29, 2014, 1:21 PM
 */

#ifndef CONTAINERS_H
#define	CONTAINERS_H

#include "CF/cf.h"

namespace redhawk {
    class DomainManagerContainer {
    public:
        DomainManagerContainer() {
                ref = CF::DomainManager::_nil();
        };
        DomainManagerContainer(CF::DomainManager_ptr domMgr) {
            ref = CF::DomainManager::_duplicate(domMgr);
        }
        CF::DomainManager_ptr getRef() {
            return ref;
        };
    private:
        CF::DomainManager_var ref;
    };
    class DeviceManagerContainer {
    public:
        DeviceManagerContainer() {
            ref = CF::DeviceManager::_nil();
        };
        DeviceManagerContainer(CF::DeviceManager_ptr devMgr) {
            ref = CF::DeviceManager::_duplicate(devMgr);
        }
        CF::DeviceManager_ptr getRef() {
            return ref;
        };
    private:
        CF::DeviceManager_var ref;
    };
    class ApplicationContainer {
    public:
        ApplicationContainer() {
            ref = CF::Application::_nil();
        };
        ApplicationContainer(CF::Application_ptr app) {
            ref = CF::Application::_duplicate(app);
        };
        CF::Application_ptr getRef() {
            return ref;
        };
    private:
        CF::Application_var ref;
    };
}

#endif	/* CONTAINERS_H */

