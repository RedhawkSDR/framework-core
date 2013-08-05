import logging

log = logging.getLogger(__name__)

class Endpoint(object):
    def disconnected(self, connectionId):
        pass

class PortEndpoint(Endpoint):
    def __init__(self, supplier, port):
        self.supplier = supplier
        self.port = port

    def getReference(self):
        return self.supplier.getPort(self.port['Port Name'])

    def getName(self):
        return '%s/%s' % (self.supplier._instanceName, self.port['Port Name'])

    def getInterface(self):
        return self.port['Port Interface']

    def hasComponent(self, component):
        return self.supplier._refid == component._refid

    def getRefid(self):
        return self.supplier._refid

    def getPortName(self):
        return self.port['Port Name']

    def disconnected(self, connectionId):
        self.supplier._disconnected(connectionId)


class ComponentEndpoint(Endpoint):
    def __init__(self, component):
        self.component = component

    def getReference(self):
        return self.component.ref

    def getName(self):
        return self.component._instanceName

    def getInterface(self):
        return 'IDL:CF/Resource:1.0'

    def hasComponent(self, component):
        return self.component._refid == component._refid

    def getRefid(self):
        return self.component._refid

    def getPortName(self):
        return None

class ServiceEndpoint(Endpoint):
    def __init__(self, service, interface):
        self.service = service
        self.interface = interface

    def getReference(self):
        return self.service.ref

    def getName(self):
        return self.service._instanceName

    def getInterface(self):
        return self.interface

    def hasComponent(self, component):
        return self.service._refid == component._refid

    def getRefid(self):
        return self.service._refid

class ConnectionManager(object):
    __instance__ = None

    @classmethod
    def instance(cls):
        if not cls.__instance__:
            cls.__instance__ = ConnectionManager()
        return cls.__instance__

    def __init__(self):
        self.__connections = {}

    def getConnections(self):
        return self.__connections

    def getConnectionsBetween(self, usesComponent, providesComponent):
        connections = {}
        for identifier, (uses, provides) in self.__connections.iteritems():
            if uses.hasComponent(usesComponent) and provides.hasComponent(providesComponent):
                connections[identifier] = (uses, provides)
        return connections

    def registerConnection(self, identifier, uses, provides):
        if identifier in self.__connections:
            log.warn("Skipping registration of duplicate connection id '%s'", identifier)
            return
        log.debug("Registering connection '%s' from %s to %s", identifier, uses, provides)
        self.__connections[identifier] = (uses, provides)

    def unregisterConnection(self, identifier):
        if not identifier in self.__connections:
            log.warn("Skipping unregistration of unknown connection id '%s'", identifier)
            return
        log.debug("Unregistering connection '%s'", identifier)
        del self.__connections[identifier]

    def breakConnection(self, identifier):
        uses, provides = self.__connections[identifier]
        log.debug("Breaking connection '%s'", identifier)
        try:
            usesPort = uses.getReference()
            usesPort.disconnectPort(identifier)
        except:
            log.warn("Ignoring exception breaking connection '%s'", identifier)
        uses.disconnected(identifier)
        provides.disconnected(identifier)

    def cleanup(self):
        for identifier in self.__connections.iterkeys():
            self.breakConnection(identifier)
        self.__connections = {}
