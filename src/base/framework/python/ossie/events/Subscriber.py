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

import sys
import time
import Queue
import copy
import threading
import logging
import traceback

from omniORB import any, URI, CORBA
from ossie.cf import CF, CF__POA
import CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA


class DefaultConsumer(CosEventComm__POA.PushConsumer):
    def __init__(self,parent):
        self.parent = parent

    def disconnect_push_consumer(self):
        if self.parent and self.parent.proxy:
            self.parent.proxy = None

    def push(self, data):
        if self.parent.dataArrivedCB != None:
            self.parent.logger.trace('Received (callback) DATA: ' + str(data))
            self.parent.dataArrivedCB( data )
        else:
            self.parent.logger.trace('Received (queue) DATA: ' + str(data))
            self.parent.events.put(data)
        

class Subscriber:
    def __init__(self, channel, dataArrivedCB=None):
        self.channel = channel
        self.proxy = None
        self.consumer = DefaultConsumer(self)
        self.dataArrivedCB=dataArrivedCB
        self.is_local = True
        self.events = Queue.Queue()
        self.my_lock = threading.Lock()
        self.logger = logging.getLogger('ossie.events.Subscriber')
        self.connect()


    def __del__(self):
        self.terminate()

    def setDataArrivedCB(self, newCB=None ):
        self.dataArrivedCB= newCB

    def terminate(self):
        if self.proxy:
            for x in range(10):
                try:
                    self.proxy.disconnect_push_supplier()
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(.01)
        self.proxy = None
        if self.is_local:
            self.consumer = None
        self.channel = None

    def getData(self):
        retval=None
        if self.dataArrivedCB != None:
            return retval;
        
        try:
            tmp = self.events.get(False,.01)
            self.logger.debug('getData: ' + str(tmp))
            retval = any.from_any(tmp)
        except:
            print traceback.print_exc()
            retval=None
        
        return retval


    def disconnect(self, retries=10, retry_wait=.01):
        if self.channel == None:
            return -1

        retval=0
        if self.proxy:
            retval=-1
            for x in range(retries):
                try:
                    self.proxy.disconnect_push_supplier()
                    retval=0
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)
        return retval


    def connect(self, retries=10, retry_wait=.01):
        
        if self.channel == None:
            return -1

        if self.proxy == None :
            for x in range(retries):
                try: 
                    admin = self.channel.for_consumers()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

            for x in range(retries):
                try: 
                    self.proxy = admin.obtain_push_supplier()
                    break
                except CORBA.COMM_FAILURE:
                    pass
            
                time.sleep(retry_wait)

        if self.consumer == None:
            self.consumer = DefaultConsumer(self)

        for x in range(retries):
            try: 
                self.proxy.connect_push_consumer( self.consumer._this() )
                break
            except CORBA.BAD_PARM:
                break
            except CORBA.COMM_FAILURE:
                pass
            except CosEventChannelAdmin.AlreadyConnected:
                break
                
            time.sleep(retry_wait)

