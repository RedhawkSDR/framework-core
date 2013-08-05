import logging
import threading
import atexit

from omniORB import CORBA
from ossie.cf import CF__POA
from ossie.utils import log4py

# Prepare the ORB
orb = CORBA.ORB_init()
poa = orb.resolve_initial_references("RootPOA")

log = logging.getLogger(__name__)

def _deviceManagerShutdown():
    devMgr = DeviceManagerStub.instance(False)
    if devMgr:
        devMgr.shutdown()

atexit.register(_deviceManagerShutdown)

class DeviceManagerStub(CF__POA.DeviceManager):
    """
    Proxy used to provide launched Devices a point with which to register
    """
    def __init__(self):
        self.__devices = {}
        self.__services = {}
        self.__lock = threading.RLock()

    _instance = None
    _instanceLock = threading.RLock()

    @classmethod
    def instance(cls, create=True):
        cls._instanceLock.acquire()
        try:
            if not cls._instance and create:
                log.debug('Launching virtual DeviceManager')
                cls._instance = DeviceManagerStub()
                cls._instance._object_id = poa.activate_object(cls._instance)
            return cls._instance
        finally:
            cls._instanceLock.release()

    def shutdown(self):
        log.debug('Shutting down virtual DeviceManager')
        poa.deactivate_object(self._object_id)

    def _registerDevice(self, identifier, device):
        if identifier in self.__devices:
            log.warning("Device '%s' already registered with virtual DeviceManager", identifier)
        log.debug("Registering device '%s' with virtual DeviceManager", identifier)
        self.__devices[identifier] = device

    def _getDeviceIdentifier(self, device):
        try:
            return device._get_identifer()
        except:
            pass
        for identifier, knownDevice in self.__devices.iteritems():
            if knownDevice._is_equivalent(device):
                return identifier
        return None

    def _unregisterDevice(self, device):
        identifier = self._getDeviceIdentifier(device)
        if not identifier:
            log.warning('Virtual DeviceManager ignoring unregistration of unidentified device')
        elif identifier not in self.__devices:
            log.warning("Device '%s' not registered with virtual DeviceManager", identifier)
        else:
            log.debug("Unregistering device '%s' from virtual DeviceManager", identifier)
            del self.__devices[identifier]

    def registerDevice(self, registeringDevice):
        try:
            identifier = registeringDevice._get_identifier()
        except:
            return
        self.__lock.acquire()
        try:
            self._registerDevice(identifier, registeringDevice)
        finally:
            self.__lock.release()

    def unregisterDevice(self, unregisteringDevice):
        self.__lock.acquire()
        try:
            self._unregisterDevice(unregisteringDevice)
        finally:
            self.__lock.release()

    def registerService(self, registeringService, name):
        self.__lock.acquire()
        try:
            if name in self.__services:
                log.warning("Service '%s' already registered with virtual DeviceManager", name)
            log.debug("Registering service '%s' with virtual DeviceManager", name)
            self.__services[name] = registeringService
        finally:
            self.__lock.release()

    def unregisterService(self, unregisteringService, name):
        self.__lock.acquire()
        try:
            if name not in self.__services:
                log.warning("Service '%s' not registered with virtual DeviceManager", name)
            else:
                del self.__services[name]
        finally:
            self.__lock.release()

    def getDevice(self, identifier):
        self.__lock.acquire()
        try:
            return self.__devices.get(identifier, None)
        finally:
            self.__lock.release()

    def getService(self, name):
        self.__lock.acquire()
        try:
            return self.__services.get(name, None)
        finally:
            self.__lock.release()
