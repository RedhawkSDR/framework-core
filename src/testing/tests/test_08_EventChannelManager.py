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
from ossie.properties import *
import threading
import time


class EventChannelManager(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        try:
            self._app.stop()
            self._app.releaseObject()
        except AttributeError:
            pass

        try:
            self._devMgr.shutdown()
        except AttributeError:
            pass

        try:
            self.terminateChild(self._devBooter)
        except AttributeError:
            pass

        try:
            self.terminateChild(self._domBooter)
        except AttributeError:
            pass

        # Do all application and node booter shutdown before calling the base
        # class tearDown, or failures will occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_ECM_CppComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM1/ECM1.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(5)
        components = app._get_registeredComponents()
        for component in components:
            print component.componentObject._get_identifier()
            if 'ECM' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        
        if stuff:
            pdict = props_to_dict(stuff)
            mlimit = pdict['msg_limit']
            mxmit = pdict['msg_xmit']
            mrecv = pdict['msg_recv']
            self.assertEquals(mlimit, mxmit )
            self.assertEquals(mlimit, mrecv )



        app.releaseObject()


    def test_ECM_PythonComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM2/ECM2.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(5)
        components = app._get_registeredComponents()
        for component in components:
            print component.componentObject._get_identifier()
            if 'ECM' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        
        if stuff:
            pdict = props_to_dict(stuff)
            mlimit = pdict['msg_limit']
            mxmit = pdict['msg_xmit']
            mrecv = pdict['msg_recv']
            self.assertEquals(mlimit, mxmit )
            self.assertEquals(mlimit, mrecv )



        app.releaseObject()


    def test_ECM_JavaComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM3/ECM3.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(5)
        components = app._get_registeredComponents()
        for component in components:
            print component.componentObject._get_identifier()
            if 'ECM' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        
        if stuff:
            pdict = props_to_dict(stuff)
            mlimit = pdict['msg_limit']
            mxmit = pdict['msg_xmit']
            mrecv = pdict['msg_recv']
            self.assertEquals(mlimit, mxmit )
            self.assertEquals(mlimit, mrecv )



        app.releaseObject()
