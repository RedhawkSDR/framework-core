import warnings

from ossie.cf import CF
from ossie.utils.notify import notification
from ossie.utils import model
from ossie.utils.weakmethod import WeakBoundMethod

from component import DomainComponent
from model import CorbaAttribute

def createDevice(profile, spd, scd, prf, deviceRef, instanceName, refid, impl=None, idmListener=None):
    """
    Factory method to create a new device instance of the most specific
    type supported by that device, according to its supported interfaces.
    """
    interfaces = set(ifc.get_repid() for ifc in scd.get_componentfeatures().get_supportsinterface())
    aggregate = 'IDL:CF/AggregateDevice:1.0' in interfaces
    if 'IDL:CF/ExecutableDevice:1.0' in interfaces:
        if aggregate:
            cls = AggregateExecutableDevice
        else:
            cls = ExecutableDevice
    elif 'IDL:CF/LoadableDevice:1.0' in interfaces:
        if aggregate:
            cls = AggregateLoadableDevice
        else:
            cls = LoadableDevice
    else:
        if aggregate:
            cls = AggregatePlainDevice
        else:
            cls = Device
    return cls(profile, spd, scd, prf, deviceRef, instanceName, refid, impl, idmListener)

class DomainDevice(DomainComponent):
    @notification
    def adminStateChanged(self, oldState, newState):
        """
        The administrative state of this device changed from 'oldState' to
        'newState'.
        """
        pass

    @notification
    def usageStateChanged(self, oldState, newState):
        """
        The usage state of this device changed from 'oldState' to 'newState'.
        """
        pass

    @notification
    def operationalStateChanged(self, oldState, newState):
        """
        The operational state of this device changed from 'oldState' to
        'newState'.
        """
        pass

    @property
    def device_ref(self):
        warnings.warn('Device.device_ref is deprecated. Device reference is automatically typed.')
        return self.ref

    @property
    def loadabledevice_ref(self):
        warnings.warn('Device.loadabledevice_ref is deprecated. Device reference is automatically typed.')
        return self.ref._narrow(CF.LoadableDevice)

    @property
    def executabledevice_ref(self):
        warnings.warn('Device.loadabledevice_ref is deprecated. Device reference is automatically typed.')
        return self.ref._narrow(CF.ExecutableDevice)

    @property
    def aggregatedevice_ref(self):
        warnings.warn('Device.aggregatedevice_ref is deprecated. Device reference is automatically typed.')
        return self.ref._narrow(CF.AggregateDevice)

    def __init__(self, profile, spd, scd, prf, instanceName, refid, impl=None, idmListener=None):
        DomainComponent.__init__(self, profile, spd, scd, prf, instanceName, refid, impl)

        # NB: The model.Device class is not directly in the hierarchy, but any
        # concrete subclass should inherit from Device or one of its subclasses.
        model.Device._buildAPI(self)

        self.__adminState = CorbaAttribute(WeakBoundMethod(self._get_adminState),
                                           WeakBoundMethod(self._set_adminState))
        self.__adminState.changed.addListener(WeakBoundMethod(self.adminStateChanged))

        self.__operationalState = CorbaAttribute(WeakBoundMethod(self._get_operationalState))
        self.__operationalState.changed.addListener(WeakBoundMethod(self.operationalStateChanged))

        self.__usageState = CorbaAttribute(WeakBoundMethod(self._get_usageState))
        self.__usageState.changed.addListener(WeakBoundMethod(self.usageStateChanged))

        self.__idmListener = idmListener

        # If the domain IDM channel is available, listen for state changes.
        if self.__idmListener:
            # Only receive notifications for this device by filtering based on
            # the identifier.
            identifier = self._get_identifier()
            def matcher(deviceId, stateChangeFrom, stateChangeTo):
                return deviceId == identifier

            self.__idmListener.administrativeStateChanged.addListener(WeakBoundMethod(self.__adminStateChangeEvent), matcher)
            self.__idmListener.operationalStateChanged.addListener(WeakBoundMethod(self.__operationalStateChangeEvent), matcher)
            self.__idmListener.usageStateChanged.addListener(WeakBoundMethod(self.__usageStateChangeEvent), matcher)

    def updateReferences(self):
        warnings.warn('Device.updateReferences() is deprecated. Device references are handled automatically')

    def __get_adminState(self):
        """
        The administrative state of this device.
        """
        return self.__adminState.value

    def __set_adminState(self, state):
        self.__adminState.value = state

    adminState = property(__get_adminState, __set_adminState)

    @property
    def usageState(self):
        """
        The usage state of this device.
        """
        return self.__usageState.value

    @property
    def operationalState(self):
        """
        The operational state of this device.
        """
        return self.__operationalState.value

    def __adminStateChangeEvent(self, deviceId, stateChangeFrom, stateChangeTo):
        self.__adminState.update(stateChangeTo)

    def __operationalStateChangeEvent(self, deviceId, stateChangeFrom, stateChangeTo):
        self.__operationalState.update(stateChangeTo)

    def __usageStateChangeEvent(self, deviceId, stateChangeFrom, stateChangeTo):
        self.__usageState.update(stateChangeTo)

    def api(self):
        super(DomainDevice,self).api()
        print
        model.Device.api(self)

class Device(DomainDevice, model.Device):
    def __init__(self, profile, spd, scd, prf, deviceRef, instanceName, refid, impl=None, idmListener=None):
        model.Device.__init__(self, deviceRef)
        DomainDevice.__init__(self, profile, spd, scd, prf, instanceName, refid, impl, idmListener)

class AggregatePlainDevice(Device, model.AggregateDevice):
    def __init__(self, *args, **kwargs):
        model.AggregateDevice.__init__(self)
        Device.__init__(self, *args, **kwargs)

class LoadableDevice(DomainDevice, model.LoadableDevice):
    def __init__(self, profile, spd, scd, prf, deviceRef, instanceName, refid, impl=None, idmListener=None):
        model.LoadableDevice.__init__(self, deviceRef)
        DomainDevice.__init__(self, profile, spd, scd, prf, instanceName, refid, impl, idmListener)

class AggregateLoadableDevice(LoadableDevice, model.AggregateDevice):
    def __init__(self, *args, **kwargs):
        model.AggregateDevice.__init__(self)
        LoadableDevice.__init__(self, *args, **kwargs)

class ExecutableDevice(DomainDevice, model.ExecutableDevice):
    def __init__(self, profile, spd, scd, prf, deviceRef, instanceName, refid, impl=None, idmListener=None):
        model.ExecutableDevice.__init__(self, deviceRef)
        DomainDevice.__init__(self, profile, spd, scd, prf, instanceName, refid, impl, idmListener)

class AggregateExecutableDevice(ExecutableDevice, model.AggregateDevice):
    def __init__(self, *args, **kwargs):
        model.AggregateDevice.__init__(self)
        ExecutableDevice.__init__(self, *args, **kwargs)
