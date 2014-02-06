# -*- coding: utf-8 -*-
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
from omniORB import any
from ossie.cf import CF
from xml.dom import minidom
import os

class ComplexApplicationFactoryTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)


    def test_collocationFailed(self):
        """
        This test exercises the collocation failover capability and exhusts all available devices to deploy collocation
        requests for the waveform.  For these scenarios the device in question is: test_collocation_device that
        manages a simple counter for the number of compnents that allocated against its capacity.  That is, when the
        component deploys it submits a  supported_components = 1 to reduce the devices deployment count.  When the
        count == the device's property setting, then a capacity failure occurrs and the component cannot be deployed
        on that specific instance.

        The waveform contains 2 collocation requests, each contains 2 NOOP_CAP components that request 1 unit of supported_components's
        capacity from the deployment device.

        Each node in this case, manages 2 test_collocation_devices each having a supported_components property == 1
        """
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node2_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_req_capacity/test_collocation_req_capacity.sad.xml")

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def test_collocationSuccess(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev2cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_req_capacity/test_collocation_req_capacity.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        ## need to check that all the comopnents were allocated to devices from test_collocation_node1_2dev2cap

        self.assertNotEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationWithComponentImplementationRollover(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev2cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_implementation_rollover/test_collocation_implementation_rollover.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())



    def test_collocationNOOPMixture(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node2_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_mix2/test_collocation_mix2.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def test_collocationNoCollocation(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node2_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_no_collocation/test_no_collocation.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationMixAndNoCollocation(self):

        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev2cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_no_collocation_mix1/test_no_collocation_mix1.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def _getBogoMips(self, device):
        return device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0].value._v


        self._domMgr.uninstallApplication(appFact._get_identifier())
