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
import scatest
from ossie.cf import CF
from omniORB import CORBA, any
import os

class PyPropertiesTest(scatest.CorbaTestCase):
    def setUp(self):
        print "-----------------------------------------"
        print os.getenv('OSSIEHOME')
        print "-----------------------------------------"
        domBooter, self._domMgr = self.launchDomainManager(debug=9)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def _launchApp (self, name):
        sadpath = "/waveforms/%s/%s.sad.xml" % (name, name)
        self._domMgr.installApplication(sadpath)
        appFact = self._domMgr._get_applicationFactories()[0]
        return appFact.create(appFact._get_name(), [], [])

    def test_sadPropertyOverride (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp("TestPythonPropsWithOverride")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # The TestPyProps component is also the assembly controller, so we can
        # query it via the application.
        prop = self._app.query([CF.DataType("DCE:897a5489-f680-46a8-a698-e36fd8bbae80[]", any.to_any(None))])[0]

        # Check that there is one value, and it matches the expected value from
        # the SAD.
        value = any.from_any(prop.value)
        self.assertEqual(len(value), 1)
        d = dict([(dt['id'], dt['value']) for dt in value[0]])
        self.assertEqual(d, {'ip_address': '127.0.0.1', 'port': 4688})

    def test_pythonPropertyTypes (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp("TestPythonPropsWithOverride")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # The TestPyProps component is also the assembly controller, so we can
        # query it via the application.
        props = self._app.query([CF.DataType("DCE:ffe634c9-096d-425b-86cc-df1cce50612f", any.to_any(None)),
                                CF.DataType("test_float", any.to_any(None)),
                                CF.DataType("test_double", any.to_any(None))])

        for prop in props:
            if prop.id == "DCE:ffe634c9-096d-425b-86cc-df1cce50612f":
                prop_struct = prop
            if prop.id == "test_float":
                prop_float = prop
            if prop.id == "test_double":
                prop_double = prop
        self.assertEqual(prop_float.value.typecode(), CORBA.TC_float)
        self.assertEqual(prop_double.value.typecode(), CORBA.TC_double)

        for sub in prop_struct.value._v:
            if sub.id == 'item4':
                sub_type_float = sub.value.typecode()
            if sub.id == 'item3':
                sub_type_double = sub.value.typecode()
        self.assertEqual(sub_type_float, CORBA.TC_float)
        self.assertEqual(sub_type_double, CORBA.TC_double)

    def test_fullQuery(self):
        dev = self._devMgr._get_registeredDevices()[0]
        results = dev.query([])
        self.assertEqual(len(results), 20)

    def test_hexProperty(self):
        dev = self._devMgr._get_registeredDevices()[0]
        prop = CF.DataType(id='hex_props',value=any.to_any(None))
        results = dev.query([prop])
        self.assertEqual(any.from_any(results[0].value)[0], 2)
        self.assertEqual(any.from_any(results[0].value)[1], 3)
        prop = CF.DataType(id='hex_prop',value=any.to_any(None))
        results = dev.query([prop])
        self.assertEqual(any.from_any(results[0].value), 2)

    def test_sequenceOperators (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp('TestPythonProps')
        self.assertNotEqual(self._app, None, "Application not created")

        # Get the value for the structsequence property. The TestPyProps component
        # is also the assembly controller, so we can query it via the application.
        id = "DCE:897a5489-f680-46a8-a698-e36fd8bbae80"
        prop = self._app.query([CF.DataType(id + '[]', any.to_any(None))])[0]
        value = any.from_any(prop.value)

        # Test the length operator.
        length = self._app.query([CF.DataType(id + '[#]', any.to_any(None))])[0]
        length = any.from_any(length.value)
        self.assertEqual(len(value), length)

        # Test key list.
        keys = self._app.query([CF.DataType(id + '[?]', any.to_any(None))])[0]
        keys = any.from_any(keys.value)
        self.assertEqual(keys, range(length))

        # Test key-value pairs.
        kvpairs = self._app.query([CF.DataType(id + '[@]', any.to_any(None))])[0]
        kvpairs = any.from_any(kvpairs.value)
        for pair in kvpairs:
            index = int(pair['id'])
            self.assertEqual(value[index], pair['value'])

        # Test indexing.
        v1 = self._app.query([CF.DataType(id + '[1]', any.to_any(None))])[0]
        v1 = any.from_any(v1.value)
        self.assertEqual(len(v1), 1)
        self.assertEqual(v1[0], value[1])

    def test_QueryBadValue(self):
        """
        Tests that invalid values in Python components do not break query()
        """
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp('TestPythonProps')
        self.assertNotEqual(self._app, None, "Application not created")

        pre_props = self._app.query([])

        # Set the internal variable to an invalid value, using runTest() to
        # bypass the normal configure protection
        self._app.runTest(0, [CF.DataType('test_float', any.to_any('bad string'))])

        # Try querying the broken property to check that the query doesn't
        # throw an unexpected exception
        self._app.query([CF.DataType('test_float', any.to_any(None))])

        # Try querying all properties to ensure that the invalid value does not
        # derail the entire query
        post_props = self._app.query([])
        self.assertEqual(len(pre_props), len(post_props))

    def _queryProperty(self, comp, propId):
        dt = comp.query([CF.DataType(propId, any.to_any(None))])[0]
        return any.from_any(dt.value)

    def _doBadConfigure(self, propId, value):
        pre_value = self._queryProperty(self._app, propId)
        props = [CF.DataType(propId, any.to_any(value))]
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, props)
        post_value = self._queryProperty(self._app, propId)
        self.assertEqual(pre_value, post_value)

    def test_ConfigureBadValue(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp('TestPythonPropsRange')
        self.assertNotEqual(self._app, None, "Application not created")

        self._doBadConfigure('my_octet', 'rejected')
        self._doBadConfigure('my_octet', [1,2,3])
        self._doBadConfigure('my_short', 'rejected')
        self._doBadConfigure('my_short', [1,2,3])

        self._doBadConfigure('seq_octet', 123.0)
        self._doBadConfigure('seq_octet', ['a', 'b', 'c'])
        self._doBadConfigure('seq_short', 1800)
        self._doBadConfigure('seq_short', 'rejected')
        self._doBadConfigure('seq_short', ['a', 'b', 'c'])

    def test_toPyValueIntString(self):
        from ossie import properties
        tests={}
        tests['octet']= [ (10,10), (0x10, 16 ), (0o10, 8 ), ('10', 10), ('0x10', 16 ), ('0o10', 8 ) ]
        tests['short']= [ (10,10), (0x10, 16 ), (0o10, 8 ), ('10', 10), ('0x10', 16 ), ('0o10', 8 ) ]
        tests['ushort']= [ (11,11), (0x11, 17 ), (0o11, 9 ), ('11', 11), ('0x11', 17 ), ('0o11', 9 ) ]
        tests['long']= [ (100,100), (0x100, 256 ), (0o100, 64 ), ('100', 100), ('0x100', 256 ), ('0o100', 64 ) ]
        tests['ulong']= [ (101,101), (0x101, 257 ), (0o101, 65 ), ('101', 101), ('0x101', 257 ), ('0o101', 65 ) ]
        tests['longlong']= [ (1000,1000), (0x1000, 4096 ), (0o1000, 512 ), ('1000', 1000), ('0x1000', 4096 ), ('0o1000', 512 ) ]
        tests['ulonglong']= [ (1001,1001), (0x1001, 4097 ), (0o1001, 513 ), ('1001', 1001), ('0x1001', 4097 ), ('0o1001', 513 ) ]

        for k,r in tests.items():
            for v in r:
               res=properties.to_pyvalue( v[0],k)
               self.assertEqual(res,v[1] )

        self.assertRaises(ValueError,properties.to_pyvalue, '0o100','float')
