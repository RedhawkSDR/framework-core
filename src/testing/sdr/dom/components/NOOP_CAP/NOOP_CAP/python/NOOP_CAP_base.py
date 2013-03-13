#!/usr/bin/env python
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

#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: NOOP_CAP.spd.xml
# Generated on: Thu Nov 01 11:01:38 EDT 2012
# Redhawk IDE
# Version:@buildLabel@
# Build id: @buildId@
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.resource import Resource

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
from ossie.cf import ExtendedCF
from omniORB import CORBA
import struct #@UnresolvedImport
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA #@UnusedImport 

NOOP = -1
NORMAL = 0
FINISH = 1
class ProcessThread(threading.Thread):
    def __init__(self, target, pause=0.0125):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.target = target
        self.pause = pause
        self.stop_signal = threading.Event()

    def stop(self):
        self.stop_signal.set()

    def updatePause(self, pause):
        self.pause = pause

    def run(self):
        state = NORMAL
        while (state != FINISH) and (not self.stop_signal.isSet()):
            state = self.target()
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                time.sleep(self.pause)

class NOOP_CAP_base(CF__POA.Resource, Resource):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatability
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            
        def initialize(self):
            Resource.initialize(self)
            
            # Instantiate the default implementations for all ports on this component
            self.port_data_in = PortBULKIODataCharIn_i(self, "data_in", self.DEFAULT_QUEUE_SIZE)

            self.port_data_out = PortBULKIODataCharOut_i(self, "data_out")

        def start(self):
            self.threadControlLock.acquire()
            try:
                Resource.start(self)
                if self.process_thread == None:
                    self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
                    self.process_thread.start()
            finally:
                self.threadControlLock.release()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            component.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            self.threadControlLock.acquire()
            try:
                process_thread = self.process_thread
                self.process_thread = None

                if process_thread != None:
                    process_thread.stop()
                    process_thread.join(self.TIMEOUT)
                    if process_thread.isAlive():
                        raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
                Resource.stop(self)
            finally:
                self.threadControlLock.release()

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            self.threadControlLock.acquire()
            try:
                Resource.releaseObject(self)
            finally:
                self.threadControlLock.release()

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.
        
        def compareSRI(self, a, b):
            if a.hversion != b.hversion:
                return False
            if a.xstart != b.xstart:
                return False
            if a.xdelta != b.xdelta:
                return False
            if a.xunits != b.xunits:
                return False
            if a.subsize != b.subsize:
                return False
            if a.ystart != b.ystart:
                return False
            if a.ydelta != b.ydelta:
                return False
            if a.yunits != b.yunits:
                return False
            if a.mode != b.mode:
                return False
            if a.streamID != b.streamID:
                return False
            if a.blocking != b.blocking:
                return False
            if len(a.keywords) != len(b.keywords):
                return False
            for keyA, keyB in zip(a.keywords, b.keywords):
                if keyA.value._t != keyB.value._t:
                    return False
                if keyA.value._v != keyB.value._v:
                    return False
            return True

        # 'BULKIO/dataChar' port
        class PortBULKIODataCharIn(BULKIO__POA.dataChar):
            """This class is a port template for the data_in port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend 
            from this class instead of the base CORBA class BULKIO__POA.dataChar.
            """
            pass


        # 'BULKIO/dataChar' port
        class PortBULKIODataCharOut(BULKIO__POA.UsesPortStatisticsProvider):
            """This class is a port template for the data_out port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend 
            from this class instead of the base CORBA class CF__POA.Port.
            """
            pass

        port_data_in = providesport(name="data_in",
                                            repid="IDL:BULKIO/dataChar:1.0",
                                            type_="data",)

        port_data_out = usesport(name="data_out",
                                            repid="IDL:BULKIO/dataChar:1.0",
                                            type_="data",)        

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.

'''provides port(s)'''


class PortBULKIODataCharIn_i(NOOP_CAP_base.PortBULKIODataCharIn):
    class linkStatistics:
        class statPoint:
            def __init__(self):
                self.elements = 0
                self.queueSize = 0.0
                self.secs = 0.0
                self.streamID = ""

        def __init__(self, port_ref):
            self.enabled = True
            self.flushTime = None
            self.historyWindow = 10
            self.receivedStatistics = []
            self.port_ref = port_ref
            self.receivedStatistics_idx = 0
            self.bitSize = struct.calcsize('c') * 8
            for i in range(self.historyWindow):
                self.receivedStatistics.append(self.statPoint())

        def setEnabled(self, enableStats):
            self.enabled = enableStats

        def update(self, elementsReceived, queueSize, streamID, flush):
            if not self.enabled:
                return

            self.receivedStatistics[self.receivedStatistics_idx].elements = elementsReceived
            self.receivedStatistics[self.receivedStatistics_idx].queueSize = queueSize
            self.receivedStatistics[self.receivedStatistics_idx].secs = time.time()
            self.receivedStatistics[self.receivedStatistics_idx].streamID = streamID
            self.receivedStatistics_idx += 1
            self.receivedStatistics_idx = self.receivedStatistics_idx%self.historyWindow
            if flush:
                self.flushTime = self.receivedStatistics[self.receivedStatistics_idx].secs

        def retrieve(self):
            if not self.enabled:
                return None

            self.runningStats = BULKIO.PortStatistics(portName=self.port_ref.name, averageQueueDepth=-1, elementsPerSecond=-1, bitsPerSecond=-1, callsPerSecond=-1, streamIDs=[], timeSinceLastCall=-1, keywords=[])

            listPtr = (self.receivedStatistics_idx + 1) % self.historyWindow    # don't count the first set of data, since we're looking at change in time rather than absolute time
            frontTime = self.receivedStatistics[(self.receivedStatistics_idx - 1) % self.historyWindow].secs
            backTime = self.receivedStatistics[self.receivedStatistics_idx].secs
            totalData = 0.0
            queueSize = 0.0
            streamIDs = []
            while (listPtr != self.receivedStatistics_idx):
                totalData += self.receivedStatistics[listPtr].elements
                queueSize += self.receivedStatistics[listPtr].queueSize
                streamIDptr = 0
                foundstreamID = False
                while (streamIDptr != len(streamIDs)):
                    if (streamIDs[streamIDptr] == self.receivedStatistics[listPtr].streamID):
                        foundstreamID = True
                        break
                    streamIDptr += 1
                if (not foundstreamID):
                    streamIDs.append(self.receivedStatistics[listPtr].streamID)
                listPtr += 1
                listPtr = listPtr%self.historyWindow

            receivedSize = len(self.receivedStatistics)
            currentTime = time.time()
            totalTime = currentTime - backTime
            if totalTime == 0:
                totalTime = 1e6
            self.runningStats.bitsPerSecond = (totalData * self.bitSize) / totalTime
            self.runningStats.elementsPerSecond = totalData / totalTime
            self.runningStats.averageQueueDepth = queueSize / receivedSize
            self.runningStats.callsPerSecond = float((receivedSize - 1)) / totalTime
            self.runningStats.streamIDs = streamIDs
            self.runningStats.timeSinceLastCall = currentTime - frontTime
            if not self.flushTime == None:
                flushTotalTime = currentTime - self.flushTime
                self.runningStats.keywords = [CF.DataType(id="timeSinceLastFlush", value=CORBA.Any(CORBA.TC_double, flushTotalTime))]

            return self.runningStats

    def __init__(self, parent, name, maxsize):
        self.parent = parent
        self.name = name
        self.queue = Queue.Queue(maxsize)
        self.port_lock = threading.Lock()
        self.stats = self.linkStatistics(self)
        self.blocking = False
        self.sriDict = {} # key=streamID, value=StreamSRI

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)

    def _get_statistics(self):
        self.port_lock.acquire()
        recStat = self.stats.retrieve()
        self.port_lock.release()
        return recStat

    def _get_state(self):
        self.port_lock.acquire()
        if self.queue.full():
            self.port_lock.release()
            return BULKIO.BUSY
        elif self.queue.empty():
            self.port_lock.release()
            return BULKIO.IDLE
        else:
            self.port_lock.release()
            return BULKIO.ACTIVE
        self.port_lock.release()
        return BULKIO.BUSY

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        activeSRIs = [self.sriDict[entry][0] for entry in self.sriDict]
        self.port_lock.release()
        return activeSRIs

    def getCurrentQueueDepth(self):
        self.port_lock.acquire()
        depth = self.queue.qsize()
        self.port_lock.release()
        return depth

    def getMaxQueueDepth(self):
        self.port_lock.acquire()
        depth = self.queue.maxsize
        self.port_lock.release()
        return depth
        
    #set to -1 for infinite queue
    def setMaxQueueDepth(self, newDepth):
        self.port_lock.acquire()
        self.queue.maxsize = int(newDepth)
        self.port_lock.release()

    def pushSRI(self, H):
        self.port_lock.acquire()
        if H.streamID not in self.sriDict:
            self.sriDict[H.streamID] = (copy.deepcopy(H), True)
            if H.blocking:
                self.blocking = True
        else:
            sri, sriChanged = self.sriDict[H.streamID]
            if not self.parent.compareSRI(sri, H):
                self.sriDict[H.streamID] = (copy.deepcopy(H), True)
                if H.blocking:
                    self.blocking = True
        self.port_lock.release()

    def pushPacket(self, data, T, EOS, streamID):
        self.port_lock.acquire()
        if self.queue.maxsize == 0:
            self.port_lock.release()
            return
        packet = None
        try:
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            sriChanged = False
            if self.sriDict.has_key(streamID):
                sri, sriChanged = self.sriDict[streamID]
                self.sriDict[streamID] = (sri, False)

            if self.blocking:
                packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                self.stats.update(len(data), float(self.queue.qsize()) / float(self.queue.maxsize), streamID, False)
                self.queue.put(packet)
            else:
                if self.queue.full():
                    try:
                        self.queue.mutex.acquire()
                        self.queue.queue.clear()
                        self.queue.mutex.release()
                    except Queue.Empty:
                        pass
                    packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, True)
                    self.stats.update(len(data), float(self.queue.qsize()) / float(self.queue.maxsize), streamID, True)
                else:
                    packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                    self.stats.update(len(data), float(self.queue.qsize()) / float(self.queue.maxsize), streamID, False)
                self.queue.put(packet)
        finally:
            self.port_lock.release()
    
    def getPacket(self):
        try:
            data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.get(block=False)
            
            if EOS: 
                if self.sriDict.has_key(streamID):
                    sri, sriChanged = self.sriDict.pop(streamID)
                    if sri.blocking:
                        stillBlock = False
                        for _sri, _sriChanged in self.sriDict.values():
                            if _sri.blocking:
                                stillBlock = True
                                break
                        if not stillBlock:
                            self.blocking = False
            return (data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed)
        except Queue.Empty:
            return None, None, None, None, None, None, None

'''uses port(s)'''


class PortBULKIODataCharOut_i(NOOP_CAP_base.PortBULKIODataCharOut):
    class linkStatistics:
        class statPoint:
            def __init__(self):
                self.elements = 0
                self.queueSize = 0.0
                self.secs = 0.0
                self.streamID = ""

        def __init__(self, port_ref):
            self.enabled = True
            self.bitSize = struct.calcsize('c') * 8
            self.historyWindow = 10
            self.receivedStatistics = {}
            self.port_ref = port_ref
            self.receivedStatistics_idx = {}

        def setEnabled(self, enableStats):
            self.enabled = enableStats

        def update(self, elementsReceived, queueSize, streamID, connectionId):
            if not self.enabled:
                return

            if self.receivedStatistics.has_key(connectionId):
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].elements = elementsReceived
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].queueSize = queueSize
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].secs = time.time()
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].streamID = streamID
                self.receivedStatistics_idx[connectionId] += 1
                self.receivedStatistics_idx[connectionId] = self.receivedStatistics_idx[connectionId]%self.historyWindow
            else:
                self.receivedStatistics[connectionId] = []
                self.receivedStatistics_idx[connectionId] = 0
                for i in range(self.historyWindow):
                    self.receivedStatistics[connectionId].append(self.statPoint())
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].elements = elementsReceived
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].queueSize = queueSize
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].secs = time.time()
                self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].streamID = streamID
                self.receivedStatistics_idx[connectionId] += 1
                self.receivedStatistics_idx[connectionId] = self.receivedStatistics_idx[connectionId] % self.historyWindow

        def retrieve(self):
            if not self.enabled:
                return

            retVal = []
            for entry in self.receivedStatistics:
                runningStats = BULKIO.PortStatistics(portName=self.port_ref.name,averageQueueDepth=-1,elementsPerSecond=-1,bitsPerSecond=-1,callsPerSecond=-1,streamIDs=[],timeSinceLastCall=-1,keywords=[])

                listPtr = (self.receivedStatistics_idx[entry] + 1) % self.historyWindow    # don't count the first set of data, since we're looking at change in time rather than absolute time
                frontTime = self.receivedStatistics[entry][(self.receivedStatistics_idx[entry] - 1) % self.historyWindow].secs
                backTime = self.receivedStatistics[entry][self.receivedStatistics_idx[entry]].secs
                totalData = 0.0
                queueSize = 0.0
                streamIDs = []
                while (listPtr != self.receivedStatistics_idx[entry]):
                    totalData += self.receivedStatistics[entry][listPtr].elements
                    queueSize += self.receivedStatistics[entry][listPtr].queueSize
                    streamIDptr = 0
                    foundstreamID = False
                    while (streamIDptr != len(streamIDs)):
                        if (streamIDs[streamIDptr] == self.receivedStatistics[entry][listPtr].streamID):
                            foundstreamID = True
                            break
                        streamIDptr += 1
                    if (not foundstreamID):
                        streamIDs.append(self.receivedStatistics[entry][listPtr].streamID)
                    listPtr += 1
                    listPtr = listPtr % self.historyWindow

                currentTime = time.time()
                totalTime = currentTime - backTime
                if totalTime == 0:
                    totalTime = 1e6
                receivedSize = len(self.receivedStatistics[entry])
                runningStats.bitsPerSecond = (totalData * self.bitSize) / totalTime
                runningStats.elementsPerSecond = totalData/totalTime
                runningStats.averageQueueDepth = queueSize / receivedSize
                runningStats.callsPerSecond = float((receivedSize - 1)) / totalTime
                runningStats.streamIDs = streamIDs
                runningStats.timeSinceLastCall = currentTime - frontTime
                usesPortStat = BULKIO.UsesPortStatistics(connectionId=entry, statistics=runningStats)
                retVal.append(usesPortStat)
            return retVal

    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outConnections = {} # key=connectionId,  value=port
        self.refreshSRI = False
        self.stats = self.linkStatistics(self)
        self.port_lock = threading.Lock()
        self.sriDict = {} # key=streamID  value=StreamSRI

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(BULKIO.dataChar)
            self.outConnections[str(connectionId)] = port
            self.refreshSRI = True
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outConnections.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)
        
    def _get_connections(self):
        currentConnections = []
        self.port_lock.acquire()
        for id_, port in self.outConnections.items():
            currentConnections.append(ExtendedCF.UsesConnection(id_, port))
        self.port_lock.release()
        return currentConnections

    def _get_statistics(self):
        self.port_lock.acquire()
        recStat = self.stats.retrieve()
        self.port_lock.release()
        return recStat

    def _get_state(self):
        self.port_lock.acquire()
        numberOutgoingConnections = len(self.outConnections)
        self.port_lock.release()
        if numberOutgoingConnections == 0:
            return BULKIO.IDLE
        else:
            return BULKIO.ACTIVE
        return BULKIO.BUSY

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        sris = []
        for entry in self.sriDict:
            sris.append(copy.deepcopy(self.sriDict[entry]))
        self.port_lock.release()
        return sris

    def pushSRI(self, H):
        self.port_lock.acquire()
        self.sriDict[H.streamID] = copy.deepcopy(H)
        try:
            try:
                for connId, port in self.outConnections.items():
                    if port != None:
                        port.pushSRI(H)
            except Exception:
                self.parent._log.exception("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)
                raise
        finally:
            self.refreshSRI = False
            self.port_lock.release()

    def pushPacket(self, data, T, EOS, streamID):
        if self.refreshSRI:
            if self.sriDict.has_key(streamID): 
                self.pushSRI(self.sriDict[streamID])

        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None:
                        port.pushPacket(data, T, EOS, streamID)
                        self.stats.update(len(data), 0, streamID, connId)
            except Exception:
                self.parent._log.exception("The call to pushPacket failed on port %s connection %s instance %s", self.name, connId, port)
                raise
            if EOS==True:
                if self.sriDict.has_key(streamID):
                    tmp = self.sriDict.pop(streamID)
        finally:
            self.port_lock.release()
 
