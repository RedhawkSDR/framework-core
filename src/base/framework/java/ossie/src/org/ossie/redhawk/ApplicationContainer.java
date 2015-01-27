package org.ossie.redhawk;

import CF.Application;

public class ApplicationContainer {
    protected CF.Application ref;
    
    public ApplicationContainer() {
        this.ref = null;
    }
    
    public ApplicationContainer(CF.Application app) {
        this.ref = app;
    }
    
    public CF.Application getRef() {
        return this.ref;
    }
};
