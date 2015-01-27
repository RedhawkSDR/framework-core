from ossie.cf import CF
from resource import Resource
import containers
from omniORB import CORBA

class Component(Resource):
    def __init__(self, identifier, execparams, propertydefs=(), loggerName=None):
        super(Component,self).__init__(identifier, execparams, propertydefs, loggerName)
        self._app = None
        
    def getApplication(self):
        return self._app
    
    def setAdditionalParameters(self, softwareProfile, application_registrar_ior):
        super(Component,self).setAdditionalParameters(softwareProfile, application_registrar_ior)
        orb = CORBA.ORB_init()
        obj = orb.string_to_object(application_registrar_ior)
        applicationRegistrar = obj._narrow(CF.ApplicationRegistrar)
        if applicationRegistrar != None:
            self._app = containers.ApplicationContainer(applicationRegistrar._get_app())
    