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

from ossie.utils.sandbox import LocalSandbox as _LocalSandbox
from ossie.utils.sandbox import IDESandbox as _IDESandbox
from io_helpers import FileSource as _FileSource
from io_helpers import FileSink as _FileSink
import os as _os
import logging as _logging
import time as _time
import threading as _threading

log = _logging.getLogger(__name__)

class ProcessingTimeout(Exception):
    def __init__(self):
        pass
    def __str__(self):
        return "Processing timed out before completion"

def proc(comp=None,_source='',_sink='',_sourceFmt=None,_sinkBlue=False,providesPortName=None,usesPortName=None,instanceName=None, refid=None, impl=None, debugger=None, window=None, execparams={}, configure={}, initialize=True, timeout=None):

    global exit_condition
    global timeout_condition

    def undo_all(undos):
        keys = undos.keys()
        keys.sort(reverse=True)
        for undo in keys:
            try:
                undoCall = getattr(undos[undo][0], undo.split(':')[-1])
                if len(undos[undo]) == 1:
                    undoCall()
                else:
                    undoCall(undos[undo][1])
            except Exception, e:
                print e
                pass
                
    def checkTimeout(timeout, undos):
        begin_time = _time.time()
        global timeout_condition
        global exit_condition
        while True:
            if timeout < (_time.time() - begin_time):
                log.warn("Timeout reached. File '"+_sink+"' is incomplete")
                timeout_condition = True
                undo_all(undos)
            _time.sleep(0.1)
            if exit_condition:
                break
        # exiting timeout thread
        
    begin_time = _time.time()
    sdrRoot = _os.environ.get('SDRROOT', None)
    log.trace("Creating local sandbox with SDRROOT '%s'", sdrRoot)
    if sdrRoot is None:
        sdrRoot = _os.getcwd()
    _sandbox = _LocalSandbox(sdrRoot)
    order = 1
    undos = {str(order)+':shutdown':(_sandbox,)}
    order += 1
            
    if comp!=None:
        try:
            _comp=_sandbox.launch(comp,instanceName, refid, impl, debugger, window, execparams, configure, initialize, timeout)
        except Exception, e:
            undo_all(undos)
            print e
            raise
    if _comp==None:
        undo_all(undos)
        return
        
    if _sourceFmt == None:
        blue = True
    else:
        blue = False
    try:
         _read=_FileSource(filename=_source,dataFormat=_sourceFmt,midasFile=blue)
         undos[str(order)+':releaseObject']=(_read,)
         order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise

    try:
         _write=_FileSink(filename=_sink,midasFile=_sinkBlue)
         undos[str(order)+':releaseObject']=(_write,)
         order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise
         
    try:
        _comp.connect(_write)
        undos[str(order)+':disconnect']=(_comp,_write)
        order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise
    try:
        _read.connect(_comp)
        undos[str(order)+':disconnect']=(_read,_comp)
        order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise
    
    try:
        _comp.start()
        undos[str(order)+':stop']=(_comp,)
        order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise
    
    try:
        _read.start()
        undos[str(order)+':stop']=(_read,)
        order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise
    
    try:
        _write.start()
        undos[str(order)+':stop']=(_write,)
        order += 1
    except Exception, e:
         undo_all(undos)
         print e
         raise
    
    exit_condition = False
    timeout_condition = False
    
    if timeout:
        timeout = timeout - (_time.time() - begin_time)
        _runThread = _threading.Thread(target=checkTimeout,args=(timeout,undos))
        _runThread.setDaemon(True)
        _runThread.start()
    
    try:
        _write.waitForEOS()
    except Exception, e:
        if not timeout_condition:
            undo_all(undos)
            print e
            raise

    exit_condition = True
    
    if not timeout_condition:
        undo_all(undos)
        return True
        
    return False

