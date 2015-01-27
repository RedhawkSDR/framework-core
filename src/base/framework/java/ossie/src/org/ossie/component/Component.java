package org.ossie.component;

import org.omg.CosNaming.NamingContext;
import org.omg.CosNaming.NamingContextHelper;

import org.omg.CORBA.ORB;
import CF.ResourceOperations;
import CF.Application;
import CF.ApplicationRegistrar;
import CF.ApplicationRegistrarHelper;
import CF.DomainManager;
import org.ossie.component.ThreadedResource;
import org.ossie.component.ThreadedComponent;
import org.ossie.redhawk.ApplicationContainer;

public abstract class Component extends ThreadedResource implements ResourceOperations,ThreadedComponent {
    protected ApplicationContainer _app;
    
    public Component() {
        super();
        this._app = null;
    }
    
    public ApplicationContainer getApplication() {
        return this._app;
    }
    
    public void setAdditionalParameters(String ApplicationRegistrarIOR) {
        super.setAdditionalParameters(ApplicationRegistrarIOR);
        final org.omg.CORBA.Object obj = this.orb.string_to_object(ApplicationRegistrarIOR);
        ApplicationRegistrar appReg = null;
        try {
            appReg = ApplicationRegistrarHelper.narrow(obj);
        } catch (Exception e) {}
        if (appReg!=null) {
            this._app = new ApplicationContainer(appReg.app());
        }
    }
};
