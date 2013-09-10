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

import logging

# Ensure that the entire ExtendedCF module is loaded, otherwise the Sandbox
# interface may be missing. Due to the way omniORB handles dependencies on
# import, if the bulkio interfaces are loaded first, the ExtendedCF module will
# only have the QueryablePort IDL.
from ossie.cf import ExtendedCF
if not hasattr(ExtendedCF, 'Sandbox'):
    import sys
    del sys.modules['ossie.cf'].ExtendedCF
    from ossie.cf import ExtendedCF

from ossie.utils.model import Resource, Device

from base import SdrRoot, Sandbox, SandboxComponent

log = logging.getLogger(__name__)

class IDESdrRoot(SdrRoot):
    def __init__(self, ideRef):
        self.__ide = ideRef
        self.__fileSystem = self.__ide._get_fileManager()

    def _fileExists(self, filename):
        return self.__fileSystem.exists(filename)

    def _readFile(self, filename):
        fileObj = self.__fileSystem.open(filename, True)
        try:
            return fileObj.read(fileObj.sizeOf())
        finally:
            fileObj.close()

    def getProfiles(self):
        return self.__ide._get_availableProfiles()

    def getLocation(self):
        return 'REDHAWK IDE virtual SDR'


class IDEMixin(object):
    def _launch(self):
        return self._sandbox._createResource(self._profile, self._refid)

    def _terminate(self):
        pass


class IDESandboxComponent(SandboxComponent, IDEMixin):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl, execparams, debugger, window):
        SandboxComponent.__init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl)
        IDEMixin.__init__(self)

        self._kick()

        self._parseComponentXMLFiles()
        self._buildAPI()
    

class IDEComponent(IDESandboxComponent, Resource):
    def __init__(self, *args, **kwargs):
        Resource.__init__(self)
        IDESandboxComponent.__init__(self, *args, **kwargs)

    def __repr__(self):
        return "<IDE component '%s' at 0x%x>" % (self._instanceName, id(self))
    

class IDEDevice(IDESandboxComponent, Device):
    def __init__(self, *args, **kwargs):
        Device.__init__(self)
        IDESandboxComponent.__init__(self, *args, **kwargs)

    def __repr__(self):
        return "<IDE device '%s' at 0x%x>" % (self._instanceName, id(self))
    

class IDESandbox(Sandbox):
    __comptypes__ = {
        'resource': IDEComponent,
        'device':   IDEDevice,
        }

    def __init__(self, ideRef):
        super(IDESandbox, self).__init__(autoInit=False)
        self.__ide = ideRef
        self.__launchedComps = {}

    def getSdrRoot(self):
        return IDESdrRoot(self.__ide)

    def _createInstanceName(self, softpkgName):
        # Use one-up counter to make component instance name unique.
        counter = len(self.__launchedComps) + 1
        while True:
            name = '%s_%d' % (softpkgName.replace('.','_'), counter)
            if name not in self.__launchedComps:
                return name
            counter += 1

    def _checkInstanceId(self, refid):
        # Ensure refid is unique.
        for component in self.__launchedComps.values():
            if refid == component._refid:
                return False
        return True

    def _createResource(self, profile, refid):
        rescFactory = self.__ide.getResourceFactoryByProfile(profile)
        log.debug("Creating resource '%s' with profile '%s'", refid, profile)
        return rescFactory.createResource(refid, [])

    def _registerComponent(self, component):
        self.__launchedComps[component._instanceName] = component

    def _unregisterComponent(self, component):
        name = component._instanceName
        if name in self.__launchedComps:
            del self.__launchedComps[name]

    def catalog(self, searchPath=None):
        if searchPath:
            log.warn("IDE sandbox does not support alternate paths")

        sdrroot = self.getSdrRoot()

        files = {}
        for profile in sdrroot.getProfiles():
            try:
                spd, scd, prf = sdrroot.readProfile(profile)
                files[str(spd.get_name())] = profile
            except:
                pass
        return files

    def getComponents(self):
        return self.__launchedComps.values()
