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


class IDEComponent(SandboxComponent, IDEMixin):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl, execparams, debugger, window):
        SandboxComponent.__init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl)
        IDEMixin.__init__(self)

        self._kick()

        self._parseComponentXMLFiles()
        self._buildAPI()
        self._ports = self._populatePorts()
    
    def __repr__(self):
        return "<IDE component '%s' at 0x%x>" % (self._instanceName, id(self))


class IDESandbox(Sandbox):
    __comptypes__ = {
        'resource':         IDEComponent,
        }

    def __init__(self, ideRef):
        self.__ide = ideRef
        self.__launchedComps = {}

    def getSdrRoot(self):
        return IDESdrRoot(self.__ide)

    def _createInstanceName(self, softpkgName):
        # TODO: Create meaningful name
        return softpkgName + '_1'

    def _checkInstanceId(self, refid):
        # TODO: Actually check
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
            spd, scd, prf = sdrroot.readProfile(profile)
            files[str(spd.get_name())] = profile
        return files

    def getComponents(self):
        return self.__launchedComps.values()
