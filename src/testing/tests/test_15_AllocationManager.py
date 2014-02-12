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
from omniORB import any as _any
from ossie.cf import CF
from ossie.cf import ExtendedCF

class AllocationManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        nb, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        self._allocMgr = self._domMgr._get_allocationMgr()

    def _packageRequest(self, requestId, props, pools=[], devices=[]):
        return CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices)

    def _tryAllocation(self, props):
        allocProps = [CF.DataType(key, _any.to_any(value)) for key, value in props.iteritems()]
        request = [self._packageRequest('test', allocProps)]
        response = self._allocMgr.allocate(request)
        if response:
            self._allocMgr.deallocate([r.allocationID for r in response])
        return len(response) == len(request)

    def test_MatchingProperties(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_Matching_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)
        self.assertTrue(len(devMgr._get_registeredDevices()) > 0)
        dev = devMgr._get_registeredDevices()[0]

        self.assertTrue(self._tryAllocation({'test_eq': 'default'}))
        self.assertFalse(self._tryAllocation({'test_eq': 'other'}))

        self.assertTrue(self._tryAllocation({'test_ne': 1}))
        self.assertFalse(self._tryAllocation({'test_ne': 0}))

        self.assertTrue(self._tryAllocation({'test_lt': 1}))
        self.assertFalse(self._tryAllocation({'test_lt': 0}))
        self.assertFalse(self._tryAllocation({'test_lt': -1}))

        self.assertTrue(self._tryAllocation({'test_le': 1}))
        self.assertTrue(self._tryAllocation({'test_le': 0}))
        self.assertFalse(self._tryAllocation({'test_le': -1}))

        self.assertFalse(self._tryAllocation({'test_gt': 1}))
        self.assertFalse(self._tryAllocation({'test_gt': 0}))
        self.assertTrue(self._tryAllocation({'test_gt': -1}))

        self.assertFalse(self._tryAllocation({'test_ge': 1}))
        self.assertTrue(self._tryAllocation({'test_ge': 0}))
        self.assertTrue(self._tryAllocation({'test_ge': -1}))

        self.assertTrue(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'OS/2'}))
        self.assertFalse(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'Linux'}))

        # Check that implementation-specific properties are overridden
        self.assertFalse(self._tryAllocation({'impl_defined': 'unknown'}))
        self.assertTrue(self._tryAllocation({'impl_defined': 'python'}))

    def test_MatchingPropertiesDCDOverride(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_MatchingDCDOverride_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)
        self.assertTrue(len(devMgr._get_registeredDevices()) > 0)
        dev = devMgr._get_registeredDevices()[0]

        self.assertFalse(self._tryAllocation({'test_eq': 'default'}))
        self.assertTrue(self._tryAllocation({'test_eq': 'override'}))

        self.assertFalse(self._tryAllocation({'test_ne': 1}))
        self.assertTrue(self._tryAllocation({'test_ne': 0}))

        self.assertTrue(self._tryAllocation({'test_lt': 2}))
        self.assertFalse(self._tryAllocation({'test_lt': 1}))
        self.assertFalse(self._tryAllocation({'test_lt': 0}))

        self.assertTrue(self._tryAllocation({'test_le': 2}))
        self.assertTrue(self._tryAllocation({'test_le': 1}))
        self.assertFalse(self._tryAllocation({'test_le': 0}))

        self.assertFalse(self._tryAllocation({'test_gt': 2}))
        self.assertFalse(self._tryAllocation({'test_gt': 1}))
        self.assertTrue(self._tryAllocation({'test_gt': 0}))

        self.assertFalse(self._tryAllocation({'test_ge': 2}))
        self.assertTrue(self._tryAllocation({'test_ge': 1}))
        self.assertTrue(self._tryAllocation({'test_ge': 0}))

        self.assertFalse(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'OS/2'}))
        self.assertTrue(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'Linux'}))

    def test_ExternalProperties(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_collocation_good_node/DeviceManager.dcd.xml', debug=self.debuglevel)

        #self._tryAllocation({ExtendedCF.WKP.OS_VERSION:'1'})
        #self._tryAllocation({'os_name':'Linux'})
        #self._tryAllocation({'supported_components': 5})
        #self._tryAllocation({'supported_components': 5, 'os_name':'Linux'})

        props = [('supported_components', 1), ('supported_components', 1)]
        allocProps = [CF.DataType(key, _any.to_any(value)) for key, value in props]
        request = [self._packageRequest('test', allocProps)]
        response = self._allocMgr.allocate(request)
        if response:
            self._allocMgr.deallocate([r.allocationID for r in response])
        return len(response) == len(request)
