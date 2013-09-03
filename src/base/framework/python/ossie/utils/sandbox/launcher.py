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
import signal
import time
import commands
import threading
import tempfile
import subprocess

from omniORB import CORBA, URI

from ossie.utils import log4py
from ossie import parsers
from ossie.utils.popen import Popen

from devmgr import DeviceManagerStub
from naming import NamingContextStub
from debugger import GDB, PDB
from terminal import XTerm

__all__ = ('ResourceLauncher', 'DeviceLauncher', 'ServiceLauncher')

# Prepare the ORB
orb = CORBA.ORB_init()
poa = orb.resolve_initial_references("RootPOA")
poa._get_the_POAManager().activate()

log = logging.getLogger(__name__)


class LocalProcess(object):
    STOP_SIGNALS = ((signal.SIGINT, 1),
                    (signal.SIGTERM, 5),
                    (signal.SIGKILL, 0))

    def __init__(self, command, arguments, environment=None, stdout=None):
        self.__command = command
        log.debug('%s %s', command, ' '.join(arguments))
        self.__process = Popen([command]+arguments, executable=command,
                               cwd=os.getcwd(), env=environment,
                               stdout=stdout, stderr=subprocess.STDOUT,
                               preexec_fn=os.setpgrp)

    def terminate(self):
        for sig, timeout in self.STOP_SIGNALS:
            try:
                log.debug('Killing process group %s with signal %s', self.__process.pid, sig)
                os.killpg(self.__process.pid, sig)
            except OSError:
                pass
            giveup_time = time.time() + timeout
            while self.__process.poll() is None:
                if time.time() > giveup_time:
                    break
                time.sleep(0.1)
            if self.__process.poll() is not None:
                break
        self.__process.wait()
        self.__process = None
    
    def command(self):
        return self.__command

    def pid(self):
        if self.__process:
            return self.__process.pid
        else:
            return None

    def isAlive(self):
        return self.__process and self.__process.poll() is None


class DebuggerProcess(object):
    def __init__(self, debugger, child):
        self.__debugger = debugger
        self.__child = child

    def terminate(self):
        self.__debugger.terminate()
        self.__child.terminate()

class LocalLauncher(object):
    def __init__(self, profile, identifier, name, sdrroot):
        self._sdrroot = sdrroot
        self._profile = profile
        self._xmlpath = os.path.dirname(self._profile)
        self._identifier = identifier
        self._name = name

    def _selectImplementation(self, spd):
        for implementation in spd.get_implementation():
            entry_point = self._getEntryPoint(implementation)
            if os.path.exists(entry_point):
                return implementation
        raise RuntimeError, "Softpkg '%s' has no usable entry point" % spd.get_name()

    def _getImplementation(self, spd, identifier):
        for implementation in spd.get_implementation():
            if implementation.get_id() == identifier:
                return implementation
        raise KeyError, "Softpkg '%s' has no implementation '%s'" % (spd.get_name(), identifier)

    def _getEntryPoint(self, implementation):
        entry_point = implementation.get_code().get_entrypoint()
        if not entry_point.startswith('/'):
            entry_point = os.path.join(self._xmlpath, entry_point)
        return entry_point

    def execute(self, spd, impl, execparams, debugger, window):
        # Find a suitable implementation.
        if impl:
            implementation = self._getImplementation(spd, impl)
        else:
            implementation = self._selectImplementation(spd)
        log.trace("Using implementation '%s'", implementation.get_id())

        # Make sure the entry point can be run.
        entry_point = self._getEntryPoint(implementation)
        if not os.access(entry_point, os.X_OK|os.R_OK):
            raise RuntimeError, "Entry point '%s' is not executable" % entry_point
        log.trace("Using entry point '%s'", entry_point)

        # Process softpkg dependencies and modify the child environment.
        environment = dict(os.environ.items())
        for dependency in implementation.get_dependency():
            for varname, pathname in self._resolveDependency(dependency):
                self._extendEnvironment(environment, varname, pathname)

        for varname in ('LD_LIBRARY_PATH', 'PYTHONPATH', 'CLASSPATH'):
            log.trace('%s=%s', varname, environment.get(varname, ''))

        # Get required execparams based on the component type
        execparams.update(self._getRequiredExecparams())

        # Convert execparams into arguments.
        arguments = []
        for name, value in execparams.iteritems():
            arguments += [name, str(value)]

        if isinstance(debugger,basestring):
            try:
                if debugger == 'pdb':
                    debugger = PDB()
                elif debugger == 'gdb':
                    debugger = GDB()
                else:
                    raise RuntimeError, 'not supported'
            except Exception, e:
                log.warning('Cannot run debugger %s (%s)', debugger, e)
                debugger = None

        if window:
            window_mode = 'monitor'
        else:
            window_mode = None

        if debugger and debugger.modifiesCommand():
            # Run the command in the debugger.
            command, arguments = debugger.wrap(entry_point, arguments)
            timeout = 60.0
            if debugger.isInteractive() and not debugger.canAttach():
                if not window:
                    window = XTerm()
                window_mode = 'direct'
        else:
            # Run the command directly.
            command = entry_point
            timeout = 10.0

        stdout = None
        if window_mode == 'monitor':
            # Open up a window for component output.
            try:
                tempdir = tempfile.mkdtemp()
                fifoname = os.path.join(tempdir, 'fifo')
                os.mkfifo(fifoname)
                window_command, window_args = window.command('/usr/bin/tail', ['-n', '+0', '-f', fifoname], self._name)
                window_proc = LocalProcess(window_command, window_args)
                stdout = open(fifoname, 'w')
                os.unlink(fifoname)
            except IOError, e:
                pass
        elif window_mode == 'direct':
            # Run the command directly in a window (typically, in the debugger).
            command, arguments = window.command(command, arguments)

        process = LocalProcess(command, arguments, environment, stdout)

        # Wait for the component to register with the virtual naming service or
        # DeviceManager.
        sleepIncrement = 0.1
        while self.getReference() is None:
            if not process.isAlive():
                raise RuntimeError, "%s '%s' terminated before registering with virtual environment" % (self._getType(), self._name)
            time.sleep(sleepIncrement)
            timeout -= sleepIncrement
            if timeout < 0:
                process.terminate()
                raise RuntimeError, "%s '%s' did not register with virtual environment"  % (self._getType(), self._name)

        # Store the CORBA reference.
        ref = self.getReference()

        # Attach a debugger to the process.
        if debugger and debugger.canAttach():
            if not window:
                window = XTerm()
            debug_command, debug_args = debugger.attach(process)
            debug_command, debug_args = window.command(debug_command, debug_args)
            debug_process = LocalProcess(debug_command, debug_args)
            process = DebuggerProcess(debug_process, process)

        return process, ref

    def _resolveDependency(self, dependency):
        softpkg = dependency.get_softpkgref()
        if not softpkg:
            return []
        filename = softpkg.get_localfile().get_name()
        log.trace("Resolving softpkg dependency '%s'", filename)
        local_filename = self._sdrroot._sdrPath('dom' + filename)
        dep_spd = parsers.spd.parse(local_filename)
        dep_impl = softpkg.get_implref()
        if dep_impl:
            impl = self._getImplementation(dep_spd, dep_impl.get_refid())
        else:
            try:
                impl = dep_spd.get_implementation()[0]
            except:
                raise RuntimeError, "Softpkg '%s' has no implementation" % dep_spd.get_name()
        log.trace("Using implementation '%s'", impl.get_id())
        dep_localfile = impl.get_code().get_localfile().name

        # Resolve nested dependencies.
        envvars = []
        for dep in impl.dependency:
            envvars.extend(self._resolveDependency(dep))

        localfile = os.path.join(os.path.dirname(local_filename), dep_localfile)
        envvars.append(self._getDependencyConfiguration(localfile))

        return envvars

    def _getDependencyConfiguration(self, localfile):
        if self._isSharedLibrary(localfile):
            return ('LD_LIBRARY_PATH', os.path.dirname(localfile))
        elif self._isPythonLibrary(localfile):
            return ('PYTHONPATH', os.path.dirname(localfile))
        elif self._isJarfile(localfile):
            return ('CLASSPATH', localfile)
        else:
            # Assume it's a set of shared libraries.
            return ('LD_LIBRARY_PATH', localfile)

    def _isSharedLibrary(self, filename):
        status, output = commands.getstatusoutput('nm ' + filename)
        return status == 0

    def _isJarfile(self, filename):
        return filename.endswith('.jar')

    def _isPythonLibrary(self, filename):
        if os.path.splitext(filename)[1] in ('.py', '.pyc', '.pyo'):
            # File is a Python module
            return True
        elif os.path.isdir(filename):
            # Check for Python package
            initpath = os.path.join(filename, '__init__.py')
            for initfile in (initpath, initpath+'c', initpath+'o'):
                if os.path.exists(initfile):
                    return True
        return False

    def _extendEnvironment(self, env, keyname, value):
        if keyname not in env:
            env[keyname] = value
        else:
            oldvalue = env[keyname].split(':')
            if value in oldvalue:
                # Path is already in list.
                return
            oldvalue.append(value)
            env[keyname] = ':'.join(oldvalue)

class ResourceLauncher(LocalLauncher):
    def __init__(self, profile, identifier, name, sdrroot):
        super(ResourceLauncher,self).__init__(profile, identifier, name, sdrroot)
        self.__namingContext = NamingContextStub()
        log.trace('Activating virtual NamingContext')
        self.__namingContextId = poa.activate_object(self.__namingContext)

    def __del__(self):
        log.trace('Deactivating virtual NamingContext')
        poa.deactivate_object(self.__namingContextId)

    def getReference(self):
        return self.__namingContext.getObject(self._name)

    def _getRequiredExecparams(self):
        return {'COMPONENT_IDENTIFIER': self._identifier,
                'NAMING_CONTEXT_IOR': orb.object_to_string(self.__namingContext._this()),
                'NAME_BINDING': self._name}

    def _getType(self):
        return 'resource'

class ServiceLauncher(LocalLauncher):
    def getReference(self):
        return DeviceManagerStub.instance().getService(self._name)

    def _getRequiredExecparams(self):
        devmgr_stub = DeviceManagerStub.instance()
        devmgr_ior = orb.object_to_string(devmgr_stub._this())

        return {'DEVICE_MGR_IOR': devmgr_ior,
                'SERVICE_NAME': self._name}

    def _getType(self):
        return 'service'

class DeviceLauncher(LocalLauncher):
    def getReference(self):
        return DeviceManagerStub.instance().getDevice(self._identifier)

    def _getRequiredExecparams(self):
        devmgr_stub = DeviceManagerStub.instance()
        devmgr_ior = orb.object_to_string(devmgr_stub._this())

        return {'DEVICE_ID': self._identifier,
                'DEVICE_LABEL': self._name,
                'DEVICE_MGR_IOR': devmgr_ior,
                'PROFILE_NAME': self._profile}

    def _getType(self):
        return 'device'
