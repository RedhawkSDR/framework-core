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
from ossie.utils import sb
from ossie.utils import type_helpers
globalsdrRoot = os.environ['SDRROOT']

class SBTestTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(True)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def assertComponentCount(self, count):
        self.assertEquals(len(sb.domainless._getSandbox().getComponents()), count)

    def tearDown(self):
        sb.domainless._getSandbox().shutdown()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    def test_catalog(self):
        # Store orig sdrroot
        sdrRoot = os.environ.pop('SDRROOT')
        
        # No SDRROOT env (should default to current directory).
        self.assertEquals(sb.catalog(), [])
        
        # Bad Sdr root env; dispose of existing sandbox instance to test.
        sb.domainless._sandbox = None
        os.environ["SDRROOT"] = ""
        self.assertEquals(sb.catalog(), [])
                
        # Good SDRROOT
        sb.setSDRROOT(sdrRoot)
        self.assertNotEquals(len(sb.catalog()), 0)
        
        # Bad search path
        self.assertEquals(len(sb.catalog("my_path")), 0)
        
        # Search path with no usable files
        self.assertEquals(len(sb.catalog("jackhammer")), 0)
        
        # Restore sdrroot
        os.environ['SDRROOT'] = sdrRoot
        
    
    def test_setSDRROOT(self):        
        # None type
        self.assertRaises(TypeError, sb.setSDRROOT, None)
        
        # Bad dir should not change root
        sdrroot = sb.getSDRROOT()
        self.assertRaises(AssertionError, sb.setSDRROOT, 'TEMP_PATH')
        self.assertEquals(sdrroot, sb.getSDRROOT())
        
        # Good dir with no dev/dom should not change root
        self.assertRaises(AssertionError, sb.setSDRROOT, 'jackhammer')
        self.assertEquals(sdrroot, sb.getSDRROOT())
        
        # New root
        sb.setSDRROOT('sdr')
        self.assertEquals(sb.getSDRROOT(), 'sdr')
        
        # Restore sdrroot
        sb.setSDRROOT(os.environ['SDRROOT'])

    
    def test_componentInit(self):        
        # Bad descriptors
        self.assertRaises(TypeError, sb.launch)
        self.assertRaises(ValueError, sb.launch, "test_name")
                
        # Make sure only one instance name and refid can be used
        comp = sb.launch(self.test_comp, "comp")
        comp.api()
        refid = comp._refid
        self.assertRaises(ValueError, sb.launch, self.test_comp, "comp")
        self.assertRaises(ValueError, sb.launch, self.test_comp, "new_comp", refid)
        
        # Only 1 component should be running
        self.assertComponentCount(1)

    def test_relativePath(self):
        comp = sb.launch('sdr/dom/components/Sandbox/Sandbox.spd.xml')
        self.assertComponentCount(1)
    
    def initValues(self, comp):
        # Init values 
        self.assertEquals(comp.my_bool_true, True)
        self.assertEquals(comp.my_bool_false, False)
        self.assertEquals(comp.my_bool_empty, None)
        self.assertEquals(comp.my_long, 10)
        self.assertEquals(comp.my_long_empty, None)
        self.assertEquals(comp.my_str, "Hello World!")
        self.assertEquals(comp.my_str_empty, None)
        self.assertEquals(comp.my_struct.bool_true, True)
        self.assertEquals(comp.my_struct.bool_false, False)
                
        self.assertEquals(comp.my_struct.bool_empty, None)
        self.assertEquals(comp.my_struct.long_s, None)
        self.assertEquals(comp.my_struct.str_s, None)
        self.assertEquals(comp.my_seq_bool[0], True)
        self.assertEquals(comp.my_seq_bool[1], False)
        self.assertEquals(comp.my_seq_str[0], "one")
        self.assertEquals(comp.my_seq_str[1], "")
        self.assertEquals(comp.my_seq_str[2], "three")
        self.assertEquals(comp.my_long_enum, None)
        self.assertEquals(comp.my_bool_enum, None)
        self.assertEquals(comp.my_str_enum, None)
        self.assertEquals(comp.my_struct.enum_long, None)
        self.assertEquals(comp.my_struct.enum_bool, None)
        self.assertEquals(comp.my_struct.enum_str, None)
    
    
    def test_simpleComp(self):       
        comp = sb.launch(self.test_comp)
        comp.api()
        
        # Check the init values
        self.initValues(comp)
        
        # Change values
        comp.my_bool_true = False
        comp.my_bool_false = True
        comp.my_long = 22
        comp.my_str = "new"
        comp.my_struct.bool_true = False
        comp.my_struct.bool_false = True
        comp.my_struct.long_s = 33
        comp.my_seq_bool[0] = False
        comp.my_seq_bool[1] = True
        comp.my_seq_str[0] = "1"
        comp.my_seq_str[1] = "2"
        comp.my_seq_str[2] = "3"
        self.assertEquals(comp.my_bool_true, False)
        self.assertEquals(comp.my_bool_false, True)
        self.assertEquals(comp.my_long, 22)
        self.assertEquals(comp.my_str, "new")
        self.assertEquals(comp.my_struct.bool_true, False)
        self.assertEquals(comp.my_struct.bool_false, True)
        self.assertEquals(comp.my_struct.long_s, 33)
        self.assertEquals(comp.my_seq_bool[0], False)
        self.assertEquals(comp.my_seq_bool[1], True)
        self.assertEquals(comp.my_seq_str[0], "1")
        self.assertEquals(comp.my_seq_str[1], "2")
        self.assertEquals(comp.my_seq_str[2], "3")

        # Checks invalid simple enumeration changes
        comp.my_long_enum = 10
        comp.my_long_enum = "one"
        comp.my_long_enum = "3"
        comp.my_long_enum = 11.1
        comp.my_long_enum = False
        self.assertEquals(comp.my_long_enum, None)
        comp.my_str_enum = 10
        comp.my_str_enum = "one"
        comp.my_str_enum = 11.1
        comp.my_str_enum = False
        self.assertEquals(comp.my_str_enum, None)
        comp.my_bool_enum = 10
        comp.my_bool_enum = "one"
        comp.my_bool_enum = 11.1
        comp.my_bool_enum = "invalid"
        self.assertEquals(comp.my_bool_enum, None)
        
        # Checks invalid struct enumeration changes
        comp.my_struct.enum_long = 10
        comp.my_struct.enum_long = "one"
        comp.my_struct.enum_long = "3"
        comp.my_struct.enum_long = 11.1
        comp.my_struct.enum_long = False
        self.assertEquals(comp.my_struct.enum_long, None)
        comp.my_struct.enum_str = 10
        comp.my_struct.enum_str = "one"
        comp.my_struct.enum_str = 11.1
        comp.my_struct.enum_str = False
        self.assertEquals(comp.my_struct.enum_str, None)
        comp.my_struct.enum_bool = 10
        comp.my_struct.enum_bool = "one"
        comp.my_struct.enum_bool = 11.1
        comp.my_struct.enum_bool = "valid"
        self.assertEquals(comp.my_struct.enum_bool, None)
        
        # Checks changes to simple enumerations
        comp.my_long_enum = "ONE"
        self.assertEquals(comp.my_long_enum, 11)
        comp.my_long_enum = "Two"
        self.assertEquals(comp.my_long_enum, 11)
        comp.my_long_enum = 33
        self.assertEquals(comp.my_long_enum, 33)
        comp.my_long_enum = 34
        self.assertEquals(comp.my_long_enum, 33)
     
        comp.my_str_enum = "ONE"
        self.assertEquals(comp.my_str_enum, "FIRST")
        comp.my_str_enum = "two"
        self.assertEquals(comp.my_str_enum, "FIRST")
        comp.my_str_enum = "SECOND"
        self.assertEquals(comp.my_str_enum, "SECOND")
        comp.my_str_enum = "third"
        self.assertEquals(comp.my_str_enum, "SECOND")
        
        comp.my_bool_enum = "VALID"
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = "false"
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = False
        self.assertEquals(comp.my_bool_enum, False)
        comp.my_bool_enum = True
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = "INVALID"
        self.assertEquals(comp.my_bool_enum, False)
        
        # Checks changes to struct enumerations
        comp.my_struct.enum_long = "ONE"
        self.assertEquals(comp.my_struct.enum_long, 1)
        comp.my_struct.enum_long = "Two"
        self.assertEquals(comp.my_struct.enum_long, 1)
        comp.my_struct.enum_long = 3
        self.assertEquals(comp.my_struct.enum_long, 3)
        comp.my_struct.enum_long = 4
        self.assertEquals(comp.my_struct.enum_long, 3)
        
        comp.my_struct.enum_str = "ONE"
        self.assertEquals(comp.my_struct.enum_str, "first")
        comp.my_struct.enum_str = "two"
        self.assertEquals(comp.my_struct.enum_str, "first")
        comp.my_struct.enum_str = "second"
        self.assertEquals(comp.my_struct.enum_str, "second")
        comp.my_struct.enum_str = "THIRD"
        self.assertEquals(comp.my_struct.enum_str, "second")
        
        comp.my_struct.enum_bool = "VALID"
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = "false"
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = False
        self.assertEquals(comp.my_struct.enum_bool, False)
        comp.my_struct.enum_bool = True
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = "INVALID"
        self.assertEquals(comp.my_struct.enum_bool, False)
        
        
        # Make sure reset works properly
        sb.reset()
        self.initValues(comp)


    def test_illegalPropertyNames(self):
        comp = sb.launch(self.test_comp)
        comp.api()
            
        self.initValues(comp)
        
        # Makes sure that no getters cause exceptions
        comp.escape__simple
        comp.escape__struct.es__1
        comp.escape__struct.es__2
        comp.escape__struct.normal
        comp.my_struct.es__3
        comp.my_struct_seq
        comp.escape__structseq
        comp.my_struct_seq[0].simp__bad
        comp.my_struct_seq[1].simp_bool
        comp.escape__structseq[0].val__1
        comp.escape__structseq[1].val_2
        
        # Makes sure that no setters cause exceptions
        comp.escape__simple = 1234
        comp.escape__struct.es__1 = 5678
        comp.escape__struct.es__2 = 4321
        comp.escape__struct.normal = 8765
        comp.my_struct.es__3 = 333
        comp.my_struct_seq[0].simp_bool = True
        comp.my_struct_seq[1].simp__bad = 11
        comp.escape__structseq[0].val_2 = "test" 
        comp.escape__structseq[1].val__1 = 22        
        
        # Makes sure that values got set properly
        self.assertEquals(comp.escape__simple, 1234)
        self.assertEquals(comp.escape__struct.es__1, 5678)
        self.assertEquals(comp.escape__struct.es__2, 4321)
        self.assertEquals(comp.escape__struct.normal, 8765)
        self.assertEquals(comp.my_struct.es__3, 333)
        self.assertEquals(comp.my_struct_seq[0].simp_bool, True)
        self.assertEquals(comp.my_struct_seq[1].simp__bad, 11)
        self.assertEquals(comp.escape__structseq[0].val_2, "test")
        self.assertEquals(comp.escape__structseq[1].val__1, 22)

    def test_loadSADFile(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp_ac.my_simple, "foo")
        self.assertEquals(comp_ac.my_seq, ["initial value"])
        self.assertEquals(comp_ac.basic_struct.some_simple, '4')
        self.assertEquals(comp.over_simple, "override")
        self.assertEquals(comp.over_struct_seq, [{'a_word': 'something', 'a_number': 1}])

    def test_loadSADFileNoOverriddenProperties(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_841_and_854/ticket_841_and_854.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('Sandbox_1')
        self.assertNotEquals(comp_ac, None)
        self.assertEquals(comp_ac.my_long, 10)

    def test_loadSADFile_overload_create(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml', props={'my_simple':'not foo','over_simple':'not override'})
        #retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml', props=[{'my_simple':'not foo'},{'over_simple':'not override'}])
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp_ac.my_simple, "not foo")
        self.assertEquals(comp_ac.my_seq, ["initial value"])
        self.assertEquals(comp_ac.basic_struct.some_simple, '4')
        self.assertEquals(comp.over_simple, "override")
        self.assertEquals(comp.over_struct_seq, [{'a_word': 'something', 'a_number': 1}])
    
    def test_simplePropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()
        
        # Test upper range
        comp.my_octet_name = 255
        comp.my_short_name = 32767
        comp.my_ushort_name = 65535
        comp.my_long_name = 2147483647
        comp.my_ulong_name = 4294967295
        comp.my_longlong_name = 9223372036854775807
        comp.my_ulonglong_name = 18446744073709551615
        self.assertEquals(comp.my_octet_name, 255)
        self.assertEquals(comp.my_short_name, 32767)
        self.assertEquals(comp.my_ushort_name, 65535)
        self.assertEquals(comp.my_long_name, 2147483647)
        self.assertEquals(comp.my_ulong_name, 4294967295)
        self.assertEquals(comp.my_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_ulonglong_name, 18446744073709551615)
        
        # Test lower range
        comp.my_octet_name = 0
        comp.my_short_name = -32768
        comp.my_ushort_name = 0
        comp.my_long_name = -2147483648
        comp.my_ulong_name = 0
        comp.my_longlong_name = -9223372036854775808
        comp.my_ulonglong_name = 0
        self.assertEquals(comp.my_octet_name, 0)
        self.assertEquals(comp.my_short_name, -32768)
        self.assertEquals(comp.my_ushort_name, 0)
        self.assertEquals(comp.my_long_name, -2147483648)
        self.assertEquals(comp.my_ulong_name, 0)
        self.assertEquals(comp.my_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_ulonglong_name, 0)
        
        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulonglong_name.configureValue, 18446744073709551616)
        
        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulonglong_name.configureValue, -1)                
        
        
    def test_structPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()
        
        # Test upper range
        comp.my_struct_name.struct_octet_name = 255
        comp.my_struct_name.struct_short_name = 32767
        comp.my_struct_name.struct_ushort_name = 65535
        comp.my_struct_name.struct_long_name = 2147483647
        comp.my_struct_name.struct_ulong_name = 4294967295
        comp.my_struct_name.struct_longlong_name = 9223372036854775807
        comp.my_struct_name.struct_ulonglong_name = 18446744073709551615
        self.assertEquals(comp.my_struct_name.struct_octet_name, 255)
        self.assertEquals(comp.my_struct_name.struct_short_name, 32767)
        self.assertEquals(comp.my_struct_name.struct_ushort_name, 65535)
        self.assertEquals(comp.my_struct_name.struct_long_name, 2147483647)
        self.assertEquals(comp.my_struct_name.struct_ulong_name, 4294967295)
        self.assertEquals(comp.my_struct_name.struct_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_struct_name.struct_ulonglong_name, 18446744073709551615)
        
        # Test lower range
        comp.my_struct_name.struct_octet_name = 0
        comp.my_struct_name.struct_short_name = -32768
        comp.my_struct_name.struct_ushort_name = 0
        comp.my_struct_name.struct_long_name = -2147483648
        comp.my_struct_name.struct_ulong_name = 0
        comp.my_struct_name.struct_longlong_name = -9223372036854775808
        comp.my_struct_name.struct_ulonglong_name = 0
        self.assertEquals(comp.my_struct_name.struct_octet_name, 0)
        self.assertEquals(comp.my_struct_name.struct_short_name, -32768)
        self.assertEquals(comp.my_struct_name.struct_ushort_name, 0)
        self.assertEquals(comp.my_struct_name.struct_long_name, -2147483648)
        self.assertEquals(comp.my_struct_name.struct_ulong_name, 0)
        self.assertEquals(comp.my_struct_name.struct_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_struct_name.struct_ulonglong_name, 0)
        
        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulonglong_name.configureValue, 18446744073709551616)
        
        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulonglong_name.configureValue, -1)       
        
        # Makes sure the struct can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        new_value = {'struct_octet': 100, 'struct_short': 101, 'struct_ushort': 102, 'struct_long': 103,
                     'struct_ulong': 104, 'struct_longlong': 105, 'struct_ulonglong': 106}
        comp.my_struct_name = new_value
        self.assertEquals(comp.my_struct_name, new_value)
        
    def test_seqPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()
        
        # Test upper and lower bounds
        comp.seq_octet_name[0] = 0
        comp.seq_octet_name[1] = 255
        comp.seq_short_name[0] = -32768
        comp.seq_short_name[1] = 32767
        comp.seq_ushort_name[0] = 0
        comp.seq_ushort_name[1] = 65535
        comp.seq_long_name[0] = -2147483648
        comp.seq_long_name[1] = 2147483647
        comp.seq_ulong_name[0] = 0
        comp.seq_ulong_name[1] = 4294967295
        comp.seq_longlong_name[0] = -9223372036854775808
        comp.seq_longlong_name[1] = 9223372036854775807
        comp.seq_ulonglong_name[0] = 0
        #comp.seq_ulonglong_name[1] = 18446744073709551615
        self.assertEquals(comp.seq_octet_name[0], 0) 
        self.assertEquals(comp.seq_octet_name[1], 255)
        self.assertEquals(comp.seq_short_name[0], -32768)
        self.assertEquals(comp.seq_short_name[1], 32767)
        self.assertEquals(comp.seq_ushort_name[0], 0)
        self.assertEquals(comp.seq_ushort_name[1], 65535)
        self.assertEquals(comp.seq_long_name[0], -2147483648)
        self.assertEquals(comp.seq_long_name[1], 2147483647)
        self.assertEquals(comp.seq_ulong_name[0], 0)
        self.assertEquals(comp.seq_ulong_name[1], 4294967295)
        self.assertEquals(comp.seq_longlong_name[0], -9223372036854775808)
        self.assertEquals(comp.seq_longlong_name[1], 9223372036854775807)
        self.assertEquals(comp.seq_ulonglong_name[0], 0)
        #self.assertEauals(comp.seq_ulonglong_name[1], 18446744073709551615)

        # Test one beyond upper bound    
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_octet.configureValue, [0, 256])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_short.configureValue, [0, 32768])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ushort.configureValue, [0, 65536])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_long.configureValue, [0, 2147483648])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulong.configureValue, [0, 4294967296])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_longlong.configureValue, [0, 9223372036854775808])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulonglong.configureValue, [0, 18446744073709551616])
        
        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_octet.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_short.configureValue, [-32769, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ushort.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_long.configureValue, [-2147483649, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulong.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_longlong.configureValue, [-9223372036854775809, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulonglong.configureValue, [-1, 0])
        
        # Tests char and octet sequences
        self.assertRaises(TypeError, comp.seq_char_name.configureValue, ['A','BB'])
        self.assertRaises(TypeError, comp.seq_char_name.configureValue, ['a', 1])
        self.assertRaises(TypeError, comp.seq_octet_name.configureValue, [1, 'a'])
        
        comp.seq_char_name[0] = 'X'
        comp.seq_char_name[1] = 'Y'
        self.assertEquals(comp.seq_char_name[0], 'X')
        self.assertEquals(comp.seq_char_name[1], 'Y')

        
    def test_structSeqPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()
        
        # Test upper and lower bounds
        comp.my_structseq_name[0].ss_octet_name = 255
        comp.my_structseq_name[1].ss_octet_name = 0
        comp.my_structseq_name[0].ss_short_name = 32767
        comp.my_structseq_name[1].ss_short_name = -32768
        comp.my_structseq_name[0].ss_ushort_name = 65535
        comp.my_structseq_name[1].ss_ushort_name = 0
        comp.my_structseq_name[0].ss_long_name = 2147483647
        comp.my_structseq_name[1].ss_long_name = -2147483648
        comp.my_structseq_name[0].ss_ulong_name = 4294967295
        comp.my_structseq_name[1].ss_ulong_name = 0
        comp.my_structseq_name[0].ss_longlong_name = 9223372036854775807
        comp.my_structseq_name[1].ss_longlong_name = -9223372036854775808
        comp.my_structseq_name[0].ss_ulonglong_name = 18446744073709551615
        comp.my_structseq_name[1].ss_ulonglong_name = 0
        self.assertEquals(comp.my_structseq_name[0].ss_octet_name, 255)
        self.assertEquals(comp.my_structseq_name[1].ss_octet_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_short_name, 32767)
        self.assertEquals(comp.my_structseq_name[1].ss_short_name, -32768)
        self.assertEquals(comp.my_structseq_name[0].ss_ushort_name, 65535)
        self.assertEquals(comp.my_structseq_name[1].ss_ushort_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_long_name, 2147483647)
        self.assertEquals(comp.my_structseq_name[1].ss_long_name, -2147483648)
        self.assertEquals(comp.my_structseq_name[0].ss_ulong_name, 4294967295)
        self.assertEquals(comp.my_structseq_name[1].ss_ulong_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_structseq_name[1].ss_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_structseq_name[0].ss_ulonglong_name, 18446744073709551615)
        self.assertEquals(comp.my_structseq_name[1].ss_ulonglong_name, 0)
        
        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ulonglong_name.configureValue, 18446744073709551616)
        
        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ulonglong_name.configureValue, -1)
        
        # Make sure entire struct seq can be set without error
        new_value = [{'ss_octet': 100, 'ss_short': 101, 'ss_ushort': 102, 'ss_long': 103, 
                      'ss_ulong': 104, 'ss_longlong': 105, 'ss_ulonglong': 106},
                     {'ss_octet': 107, 'ss_short': 108, 'ss_ushort': 109, 'ss_long': 110, 
                      'ss_ulong': 111, 'ss_longlong': 112, 'ss_ulonglong': 113}]
        comp.my_structseq_name = new_value
        self.assertEqual(comp.my_structseq_name, new_value)
        
        # Make sure individual structs can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        for item in new_value:
            for name in item.iterkeys():
                item[name] = item[name] + 100
        comp.my_structseq_name[0] = new_value[0]
        comp.my_structseq_name[1] = new_value[1]
        self.assertEqual(comp.my_structseq_name, new_value)
        
    def test_readOnlyProps(self):
        comp = sb.launch('Sandbox')
        comp.api()
        
        # Properties should be able to be read, but not set, and all should throw the saem exception
        exception = None
        comp.readonly_simp
        try:
            comp.readonly_simp = 'bad'
        except Exception, e:
            self.assertEquals(type(e), Exception)    
        else:
            self.fail('Expected exception to be thrown for read only property')
        
        # Makes sure both ways to set struct members throws same error
        comp.readonly_struct.readonly_struct_simp
        try:
            comp.readonly_struct.readonly_struct_simp = 'bad'
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_struct = {'readonly_struct_simp':'bad'}
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
            
        # Makes sure both ways to set seqs throws same error
        comp.readonly_seq[0]
        comp.readonly_seq[1]
        try:
            comp.readonly_seq[1] = 'bad'
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_seq = ['bad1', 'bad2']
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        
        # Make sure all ways to set structseq throws the same error
        comp.readonly_structseq[0].readonly_s
        comp.readonly_structseq[1].readonly_s
        try: 
            comp.readonly_structseq[0].readonly_s = 'bad'
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try: 
            comp.readonly_structseq[1] = {'readonly_s':'bad'}
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try: 
            comp.readonly_structseq = [{'readonly_s':'bad1'}, {'readonly_s':'bad2'}]
        except Exception, e:
            self.assertEquals(type(e), Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Nothing should have changed
        self.assertEquals(comp.readonly_simp, 'Read only simple prop')
        self.assertEquals(comp.readonly_struct.readonly_struct_simp, 'read only struct property')
        self.assertEquals(comp.readonly_seq[0], 'read only')
        self.assertEquals(comp.readonly_seq[1], 'sequence property')
        self.assertEquals(comp.readonly_structseq[0].readonly_s, 'read only')
        self.assertEquals(comp.readonly_structseq[1].readonly_s, 'struct seq property')
        

    def test_Services(self):
        service = sb.launch(sb.getSDRROOT() + '/dev/services/BasicService_java/BasicService_java.spd.xml')
        comp = sb.launch('ServiceComponent')
        comp.connect(service)
        self.assertEquals(len(sb.domainless.ConnectionManager.instance().getConnections().keys()), 1)

        # Check that all the parameters got set correctly
        props = service.query([])
        d = dict([(p.id, any.from_any(p.value)) for p in props])
        self.assertEqual(d["SERVICE_NAME"], "BasicService_java_1")
        self.assertEqual(d["PARAM1"], "ABCD")
        self.assertEqual(d["PARAM2"], 42)
        self.assertAlmostEqual(d["PARAM3"], 3.1459)
        self.assertEqual(d["PARAM4"], False)
        self.assertEqual(d["PARAM5"], "Hello World")
        self.assertEqual(d.has_key("PARAM6"), False)


        #TODO if BULKIO ever gets folded into core framework these tests can be used
        # to add them proper components must be created
        # 1 with multiple good connections
        # 1 with no good connections
        # 1 with a good connection

#    def test_connections(self):
#        a = sb.launch(self.test_comp)
#        b = sb.launch("SandBoxTest2")
#        no_connections = sb.launch("SimpleComponent")
#        multiple_connections = sb.launch("SandBoxTestMultipleConnections")
#
#        names = [a._componentName,              \
#                 b._componentName,              \
#                 no_connections._componentName, \
#                 multiple_connections._componentName]
#
#        # Poor args
#        self.assertRaises(AssertionError, a.connect, b, "")
#        self.assertRaises(AssertionError, a.connect, b, None, "")
#        self.assertRaises(AssertionError, a.connect, b, "", "")
#        self.assertRaises(AssertionError, a.connect, b, "long_in", "")
#        self.assertRaises(AssertionError, a.connect, b, "", "long_out")
#        self.assertRaises(AssertionError, a.connect, no_connections)
#        
#        # Good connections
#        self.assertEquals(a.connect(b), True)
#        self.assertEquals(a.connect(b, "long_in"), True)
#        self.assertEquals(a.connect(b, None, "long_out"), True)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 3)
#            
#        # Disconnect
#        a.disconnect(no_connections)
#        a.disconnect(None)
#        self.assertEquals(len(sb.domainless._currentStatep['Component Connections'].keys()), 3)
#        a.disconnect(b)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 0)
#        
#        # Makes sure that connection keys are unique
#        self.assertEquals(a.connect(b, "long_in", "long_out", "my_conn"), True)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 1)
#        self.assertRaises(AssertionError, a.connect, b, "long_in", "long_out", "my_conn")
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 1)
#    
#        a.disconnect(b)
#        
#        # Multiple good connections
#        self.assertRaises(AssertionError, a.connect, multiple_connections)
#        
#        #### TEST for a.connectd diff ports BUG
#        ###TODO this test will fail until Issue #149 is fixed
#        #self.assertRaises(AssertionError, a.connect, multiple_connections, "long_in", "short_out")
#    
#        # Tests getComponents()
#        self.assertEquals(sb.getComponent(None), None)
#        self.assertEquals(sb.getComponent(""), None)
#        for key in sb.domainless._currentState['Components Running'].keys():
#            temp = sb.getComponent(key)
#            self.assertNotEquals(temp, None)
#            self.assertEquals(temp._componentName in names, True)  
            
    

