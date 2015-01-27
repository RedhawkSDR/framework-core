package org.ossie.redhawk;

import CF.DomainManager;

public class DomainManagerContainer {
    protected CF.DomainManager ref;
    
    public DomainManagerContainer() {
        this.ref = null;
    }
    
    public DomainManagerContainer(CF.DomainManager domMgr) {
        this.ref = domMgr;
    }
    
    public CF.DomainManager getRef() {
        return this.ref;
    }
};
