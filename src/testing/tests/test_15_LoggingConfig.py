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
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import CORBA
import struct
from ossie.utils import sb

class CppPropertiesTest(scatest.CorbaTestCase):
    def setUp(self):
        self.comp = sb.launch('CppCallbacks')

        
    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_SetLogLevel(self):

        self.comp.ref.setLogLevel( "", CF.LogLevels.INFO );

        
if __name__ == "__main__":
  # Run the unittests
  unittest.main()
