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

from omniORB import any
import unittest
import scatest
import time
from ossie.utils import redhawk

class RedhawkModuleTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=9)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml", debug=9)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        
    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        
    def test_createApplication(self):
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)
        
        app2 = self._rhDom.createApplication("TestCppProps")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 2)
        self.assertEquals(len(self._rhDom.apps), 2)
        
        app.releaseObject()
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)
        
        # Use exit functions from module to release other launched app
        redhawk.core._cleanUpLaunchedApps()
        self.assertEquals(len(self._rhDom.apps), 0)
        self.assertEquals(len(self._rhDom._get_applications()), 0)

        
    def test_largeShutdown(self):
        for i in range(16):
            self._rhDom.createApplication('TestCppProps')
            
        self.assertEquals(len(self._rhDom._get_applications()), 16)
        self.assertEquals(len(self._rhDom.apps), 16)
        
        apps = self._rhDom.apps
        
        for a in apps:
            a.start()
            a.stop()
            a.releaseObject()
            
        self.assertEquals(len(self._rhDom._get_applications()), 0)
        self.assertEquals(len(self._rhDom.apps), 0)

    def test_appListSync(self):
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)
        
        # Make sure that a list created outside of the redhawk module still updates the app list inside
        self._domMgr.installApplication('/waveforms/TestCppProps/TestCppProps.sad.xml')
        appFact = self._domMgr._get_applicationFactories()[0]
        app2 = appFact.create(appFact._get_name(), [], [])
        self.assertEquals(len(self._rhDom.apps), 2)
        
        app2.releaseObject()
        app.releaseObject()
        self.assertEquals(len(self._rhDom.apps), 0)
        self.assertEquals(len(self._rhDom._get_applications()), 0)
             
