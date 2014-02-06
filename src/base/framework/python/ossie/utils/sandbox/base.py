#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import os
import logging
import copy

from ossie import parsers
from ossie.cf import CF
from ossie.utils import log4py
from ossie.utils.model import PortSupplier, PropertySet, ComponentBase
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

log = logging.getLogger(__name__)

class SdrRoot(object):
    def readProfile(self, profile):
        xmlPath = os.path.dirname(profile)
        spd = parsers.spd.parseString(self._readFile(profile))

        # Parse the softpkg SCD, if available.
        if spd.get_descriptor():
            scdFile = os.path.join(xmlPath, str(spd.get_descriptor().get_localfile().get_name()))
            log.trace("Softpkg '%s' has SCD '%s'", spd.get_name(), scdFile)
            scd = parsers.scd.parseString(self._readFile(scdFile))
        else:
            log.trace("Softpkg '%s' has no SCD", spd.get_name())
            scd = None

        # If the softpkg has a PRF, parse that as well.
        if spd.get_propertyfile():
            prfFile = os.path.join(xmlPath, str(spd.get_propertyfile().get_localfile().get_name()))
            log.trace("Softpkg '%s' has PRF '%s'", spd.get_name(), prfFile)
            prf = parsers.prf.parseString(self._readFile(prfFile))
        else:
            log.trace("Softpkg '%s' has no PRF", spd.get_name())
            prf = None

        return spd, scd, prf

    def findProfile(self, descriptor):
        if self._fileExists(descriptor):
            try:
                spd = parsers.spd.parseString(self._readFile(descriptor))
                log.trace("Descriptor '%s' is file name", descriptor)
                return self._sdrPath(descriptor)
            except:
                pass
        for profile in self.getProfiles():
            try:
                spd = parsers.spd.parseString(self._readFile(profile))
                if spd.get_name() == descriptor:
                    log.trace("Softpkg name '%s' found in '%s'", descriptor, profile)
                    return profile
            except:
                log.warning('Could not parse %s', profile)
                continue

        raise ValueError, "'%s' is not a valid softpkg name or SPD file" % descriptor


class Sandbox(object):
    def __init__(self, autoInit=True):
        self._autoInit = autoInit

    def start(self):
        log.debug('start()')
        for component in self.getComponents():
            if not component:
                continue
            log.debug("Starting component '%s'", component._instanceName)
            component.start()

    def stop(self):
        log.debug('stop()')
        for component in self.getComponents():
            if not component:
                continue
            log.debug("Stopping component '%s'", component._instanceName)
            try:
                component.stop()
            except Exception, e:
                pass

    def reset(self):
        for component in self.getComponents():
            # Bring down current component process and re-launch it.
            component.reset()

    def launch(self, descriptor, instanceName=None, refid=None, impl=None,
               debugger=None, window=None, execparams={}, configure={},
               initialize=True, timeout=None):
        sdrRoot = self.getSdrRoot()

        # Parse the component XML profile.
        profile = sdrRoot.findProfile(descriptor)
        spd, scd, prf = sdrRoot.readProfile(profile)
        name = spd.get_name()

        if not scd:
            raise RuntimeError, 'Cannot launch softpkg with no SCD'

        if not instanceName:
            instanceName = self._createInstanceName(name)
        elif not self._checkInstanceName(instanceName):
            raise ValueError, "User-specified instance name '%s' already in use" % (instanceName,)

        if not refid:
            refid = str(uuid4())
        elif not self._checkInstanceId(refid):
            raise ValueError, "User-specified identifier '%s' already in use" % (refid,)

        # Determine the class for the component type and create a new instance.
        comptype = scd.get_componenttype()
        if comptype not in self.__comptypes__:
            raise NotImplementedError, "No support for component type '%s'" % comptype
        clazz = self.__comptypes__[comptype]
        comp = clazz(self, profile, spd, scd, prf, instanceName, refid, impl, execparams, debugger, window, timeout)

        # Services don't get initialized or configured
        if comptype == 'service':
            return comp

        # Initialize the component unless asked not to (if the subclass has not
        # disabled automatic initialization).
        if initialize and self._autoInit:
            comp.initialize()
        
        # Configure component with default values unless requested not to (e.g.,
        # when launched from a SAD file).
        if configure is not None:
            # Make a copy of the default properties, and update with the passed-in
            # properties
            initvals = copy.deepcopy(comp._configRef)
            initvals.update(configure)
            try:
                comp.configure(initvals)
            except:
                log.exception('Failure in component configuration')
        return comp


class SandboxComponent(ComponentBase):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl):
        super(SandboxComponent,self).__init__(spd, scd, prf, instanceName, refid, impl)
        self._sandbox = sandbox
        self._profile = profile
        self._componentName = spd.get_name()
        self._configRef = {}
        for prop in self._getPropertySet(modes=('readwrite', 'writeonly'), includeNil=False):
            if prop.defValue is None:
                continue
            self._configRef[str(prop.id)] = prop.defValue

        self.__ports = None

    def _readProfile(self):
        sdrRoot = self._sandbox.getSdrRoot()
        self._spd, self._scd, self._prf = sdrRoot.readProfile(self._profile)

    def _kick(self):
        self.ref = self._launch()
        self._sandbox._registerComponent(self)

    @property
    def _ports(self):
        if self.__ports == None:
            self.__ports = self._populatePorts()
        return self.__ports
        
    def reset(self):
        self.releaseObject()
        self._readProfile()
        self._kick()
        self.initialize()
        self._parseComponentXMLFiles()
        self._buildAPI()
        # Clear cached ports list
        self.__ports = None

    def releaseObject(self):
        # Break any connections involving this component.
        manager = ConnectionManager.instance()
        for identifier, (uses, provides) in manager.getConnections().items():
            if uses.hasComponent(self) or provides.hasComponent(self):
                manager.breakConnection(identifier)
                manager.unregisterConnection(identifier)
        self._sandbox._unregisterComponent(self)
        super(SandboxComponent,self).releaseObject()

    def api(self):
        '''
        Inspect interfaces and properties for the component
        '''
        print "Component [" + str(self._componentName) + "]:"
        PortSupplier.api(self)
        PropertySet.api(self)
