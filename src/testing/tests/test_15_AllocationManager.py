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
        super(AllocationManagerTest,self).setUp()
        nb, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        self._allocMgr = self._domMgr._get_allocationMgr()

    def _packageRequest(self, requestId, props, pools=[], devices=[]):
        return CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices)

    def _formatProperties(self, props):
        return [CF.DataType(key, _any.to_any(value)) for key, value in props.iteritems()]

    def _tryAllocation(self, props):
        request = [self._packageRequest('test', self._formatProperties(props))]
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

    def test_MultipleRequests(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml', debug=self.debuglevel)

        # Try two requests that should succeed
        props = self._formatProperties({'simple_alloc': 1})
        request = [self._packageRequest('test1', props), self._packageRequest('test2', props)]
        response = self._allocMgr.allocate(request)
        self.assertEqual(len(request), len(response))
        self._allocMgr.deallocate([r.allocationID for r in response])

        # The second request should fail
        props = self._formatProperties({'simple_alloc': 8})
        request = [self._packageRequest('test1', props), self._packageRequest('test2', props)]
        response = self._allocMgr.allocate(request)
        good_requests = [r.requestID for r in response]
        self.assertTrue(len(request) > len(response))
        self.assertTrue('test1' in good_requests)
        self.assertFalse('test2' in good_requests)
        self._allocMgr.deallocate([r.allocationID for r in response])

        # The first and second requests should fail, but the third should succeed
        bad_props = {'simple_alloc': 12}
        good_props = {'simple_alloc': 8}
        request = [('test1', bad_props), ('test2', bad_props), ('test3', good_props)]
        request = [self._packageRequest(k, self._formatProperties(v)) for k, v in request]
        response = self._allocMgr.allocate(request)
        good_requests = [r.requestID for r in response]
        self.assertTrue(len(request) > len(response))
        self.assertEqual(good_requests, ['test3'])
        self._allocMgr.deallocate([r.allocationID for r in response])

        # Ensure that different requests can be allocated to different devices
        request = [('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]
        request = [self._packageRequest(k, self._formatProperties(v)) for k, v in request]
        response = dict((r.requestID, r) for r in self._allocMgr.allocate(request))
        self.assertEqual(len(request), len(response))
        self.assertFalse(response['external'].allocatedDevice._is_equivalent(response['matching'].allocatedDevice))
        self._allocMgr.deallocate([r.allocationID for r in response.values()])
