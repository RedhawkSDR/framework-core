from ossie.utils.model import ComponentBase, Resource, PropertySet, PortSupplier

class DomainComponent(ComponentBase):
    def __init__(self, profile, spd, scd, prf, instanceName, refid, impl):
        super(DomainComponent,self).__init__(spd, scd, prf, instanceName, refid, impl)
        self._profile = profile
        self.ports = []

        try:
            self.name = str(spd.get_name())
            self._parseComponentXMLFiles()
            self._buildAPI()
            if self.ref != None:
                self.ports = self._populatePorts()
        except Exception, e:
            print "Component:__init__() ERROR - Failed to instantiate component " + str(self.name) + " with exception " + str(e)

    #####################################

    def api(self, showComponentName=True, showInterfaces=True, showProperties=True, externalPropInfo=None):
        '''
        Inspect interfaces and properties for the component
        '''
        className = self.__class__.__name__
        if showComponentName == True:
            print className+" [" + str(self.name) + "]:"
        if showInterfaces == True:
            PortSupplier.api(self)
        if showProperties == True and self._propertySet != None:
            PropertySet.api(self, externalPropInfo)


class Component(DomainComponent, Resource):
    """
    This representation provides a proxy to a running component. The CF::Resource api can be accessed
    directly by accessing the members of the class
    
    A simplified access to properties is available through:
        Component.<property id> provides read/write access to component properties
    
    """
    def __init__(self, profile, spd, scd, prf, objref, instanceName, refid, impl=None):
        Resource.__init__(self, objref)
        DomainComponent.__init__(self, profile, spd, scd, prf, instanceName, refid, impl)
