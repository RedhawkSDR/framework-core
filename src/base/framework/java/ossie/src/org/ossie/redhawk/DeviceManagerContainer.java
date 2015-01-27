package org.ossie.redhawk;

import CF.DeviceManager;

public class DeviceManagerContainer {
    protected CF.DeviceManager ref;
    
    public DeviceManagerContainer() {
        this.ref = null;
    }
    
    public DeviceManagerContainer(CF.DeviceManager devMgr) {
        this.ref = devMgr;
    }
    
    public CF.DeviceManager getRef() {
        return this.ref;
    }
};
