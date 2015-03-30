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
import logging
import traceback

from omniORB import any, URI, CORBA
import CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA


class DefaultSupplier(CosEventComm__POA.PushSupplier):
    def __init__(self,parent):
        self.parent = parent

    def disconnect_push_supplier(self):
        if self.parent and self.parent.proxy:
            self.parent.proxy = None
        pass

class Publisher:
    def __init__(self, channel, supplier=None ):
        self.channel = channel
        self.is_local = True
        self.supplier = supplier
        self.proxy = None
        if supplier == None:
            self.supplier = DefaultSupplier(self)
        self.logger = logging.getLogger("ossie.events.Publisher")
            
        self.connect()


    def __del__(self):
        self.terminate()


    def terminate(self):
        if self.proxy:
            for x in range(10):
                try:
                    self.proxy.disconnect_push_consumer()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(.01)

        self.proxy = None
        if self.is_local:
            self.supplier = None
        self.channel = None


    def push(self,data ):
        retval=0
        edata=data
        if not isinstance(data, CORBA.Any):
            edata = any.to_any(data)
                            
        try:
            if self.proxy != None:
                self.proxy.push(edata)
        except:
            traceback.print_exc()
            retval=-1
        
        return retval


    def disconnect(self, retries=10, retry_wait=.01):
        retval=0
        if self.channel == None:
            return retval

        if self.proxy:
            retval=-1
            for x in range(retries):
                try:
                    self.proxy.disconnect_push_consumer()
                    retval=0
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

        return retval


    def connect(self, retries=10, retry_wait=.01):
        retval=-1
        
        if self.channel == None:
            return retval
    
        if self.proxy == None:
            self.logger.debug("Getting supplier object")
            for x in range(retries):
                try: 
                    supplier_admin = self.channel.for_suppliers()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

            for x in range(retries):
                try: 
                    self.proxy = supplier_admin.obtain_push_consumer()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

        if self.supplier == None and self.is_local == True :
            self.supplier = DefaultSupplier(self)

        self.logger.debug("Assign default supplier to channel")
        for x in range(retries):
            try: 
                self.proxy.connect_push_supplier( self.supplier._this() )
                retval=0
                break
            except CORBA.BAD_PARM:
                break
            except CORBA.COMM_FAILURE:
                pass
            except CosEventChannelAdmin.AlreadyConnected:
                retval=0
                break
                
            time.sleep(retry_wait)

        return retval
