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

import unittest
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF
from xml.dom import minidom
import os
import time

def launchDomain(number, root):
    domainName = scatest.getTestDomainName() + '_' + str(number)
    _domainBooter = scatest.spawnNodeBooter(dmdFile='', domainname=domainName)
    while _domainBooter.poll() == None:
        _domainManager = None
        try:
            _domainManager = root.resolve(URI.stringToName("%s/%s" % (domainName, domainName)))._narrow(CF.DomainManager)
        except:
            _domainManager = None
        if _domainManager:
            break
        time.sleep(0.1)
    return (_domainBooter, _domainManager)

class ComplexApplicationFactoryTest(scatest.CorbaTestCase):
    def setUp(self):
        (self._domainBooter_1, self._domainManager_1) = launchDomain(1, self._root)
        (self._domainBooter_2, self._domainManager_2) = launchDomain(2, self._root)

    def tearDown(self):
        if self._domainBooter_1:
            self.terminateChild(self._domainBooter_1)
        if self._domainBooter_2:
            self.terminateChild(self._domainBooter_2)

    def test_MultipleDomainDeployment(self):
        self.assertEqual(len(self._domainManager_1._get_applicationFactories()), 0)
        self.assertEqual(len(self._domainManager_2._get_applicationFactories()), 0)
        
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1, debug=self.debuglevel)
        self.assertNotEqual(execDevNode1, None)

        self._domainManager_1.installApplication("/waveforms/CommandWrapperUsesDevice/CommandWrapper.sad.xml")
        appFact = self._domainManager_1._get_applicationFactories()[0]
        
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2, debug=self.debuglevel)
        self.assertNotEqual(basicDevNode1, None)
        
        self.assertEqual(len(self._domainManager_1._get_deviceManagers()),1)
        self.assertEqual(len(self._domainManager_2._get_deviceManagers()),1)
        
        props=[CF.DataType(id='DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f',value=any.to_any(None))]
        value_2 = any.from_any(self._domainManager_2._get_deviceManagers()[0]._get_registeredDevices()[0].query(props)[0].value)
        
        self.assertEqual(len(self._domainManager_1._get_remoteDomainManagers()), 0)
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)
        self.assertEqual(len(self._domainManager_1._get_remoteDomainManagers()), 1)

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!
        self.assertEqual(len(self._domainManager_1._get_applications()), 1)
        app.stop()
        app.releaseObject()

        self.assertEqual(len(self._domainManager_1._get_applicationFactories()), 1)
        self.assertEqual(len(self._domainManager_1._get_applications()), 0)
        self._domainManager_1.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(self._domainManager_1._get_applicationFactories()), 0)

        value_1 = any.from_any(self._domainManager_2._get_deviceManagers()[0]._get_registeredDevices()[0].query(props)[0].value)
        self.assertEquals(value_1, value_2)

if __name__ == "__main__":
  # Run the unittests
  unittest.main()
