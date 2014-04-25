# -*- coding: utf-8 -*-
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
import os
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import logging
import threading

from xml.dom import minidom

from omniORB import CORBA, any
from ossie.cf import CF

from ossie.utils import redhawk,prop_helpers

from properties import *
from browsewindowbase import BrowseWindowBase
import installdialog as installdialog
import propertydialog as propertydialog
from ossie.utils.redhawk.channels import IDMListener, ODMListener
from ossie.utils.weakmethod import WeakBoundMethod
import Queue

def buildDevSeq(dasXML, fs):
    _xmlFile = fs.open(str(dasXML), True)
    buff = _xmlFile.read(_xmlFile.sizeOf())
    _xmlFile.close()

    das = minidom.parseString(buff)
    ds = []
    deviceAssignmentTypeNodeList = das.getElementsByTagName("deviceassignmenttype")
    for node in deviceAssignmentTypeNodeList:
        componentid = node.getElementsByTagName("componentid")[0].firstChild.data
        assigndeviceid = node.getElementsByTagName("assigndeviceid")[0].firstChild.data
        ds.append( CF.DeviceAssignmentType(str(componentid),str(assigndeviceid)) )
    return ds


def clearChildren (item):
    while item.childCount() > 0:
        item.removeChild(item.child(0))


TYPE_MAP = { "boolean"  : CORBA.TC_boolean,
             "char"     : CORBA.TC_char,
             "double"   : CORBA.TC_double,
             "float"    : CORBA.TC_float,
             "short"    : CORBA.TC_short,
             "long"     : CORBA.TC_long,
             "longlong" : CORBA.TC_longlong,
             "objref"   : CORBA.TC_Object,
             "octet"    : CORBA.TC_octet,
             "string"   : CORBA.TC_string,
             "ulong"    : CORBA.TC_ulong,
             "ushort"   : CORBA.TC_ushort }

def toAny (value, type):
    if type in ( "short", "long", "octet", "ulong", "ushort" ):
        value = int(value)
    elif type in ( "float", "double" ):
        value = float(value)
    elif type == "boolean":
        value = str(value.lower()) in ( "true", "1" )
    elif type == "longlong":
        value = long(value)
    elif type == "char":
        value = value[0]
    return CORBA.Any(TYPE_MAP[type], value)

class BrowseWindow(BrowseWindowBase):

    def __init__(self, domainName=None, verbose=False,
                 parent = None,name = None,fl = 0):
        redhawk.setTrackApps(False)
        self.verbose = verbose
        self.currentDomain = ""
        self.apps = []
        self.devMgrs = []
        self.odmListener = None
        
        self.log = logging.getLogger(BrowseWindow.__name__)

        if not domainName:
            domainName = self.findDomains()
        BrowseWindowBase.__init__(self,parent,name,fl, domainName)
        self._requests = Queue.Queue()
        self.worker = self.WorkThread(self._requests)
        if not domainName:
            return
        self.debug('Domain:', domainName)
        self.connect(self.worker, SIGNAL('refreshApplications()'), self.refreshApplications)
        self.connect(self.worker, SIGNAL('refreshDeviceManagers()'), self.refreshDeviceManagers)
        self.connect(self.worker, SIGNAL('parseDomainManager()'), self.parseDomainManager)
        self.connect(self.worker, SIGNAL('parseApplications()'), self.parseApplications)
        self.connect(self.worker, SIGNAL('parseAllDeviceManagers()'), self.parseAllDeviceManagers)
        self.connect(self.worker, SIGNAL('setupDomain()'), self.setupDomain)
        self.connect(self.worker, SIGNAL('cleardomMgrItem()'), self.cleardomMgrItem)
        self.connect(self.worker, SIGNAL('cleardevMgrItem()'), self.cleardevMgrItem)
        self.connect(self.worker, SIGNAL('clearappsItem()'), self.clearappsItem)
        self.connect(self.worker, SIGNAL('setWindowTitle(QString)'), self.setWindowTitle)
        self.connect(self.worker, SIGNAL('releaseApplication(QString)'), self.releaseApplication)
        self.connect(self.worker, SIGNAL('shutdownDeviceManager(QString)'), self.shutdownDeviceManager)
        self.worker.start()
        self.updateDomain(domainName)

    class WorkThread(QThread):
        def __init__(self, request_queue):
            QThread.__init__(self)
            self.requests = request_queue
            self.exit = False

        def end(self):
            self.exit = True

        def run(self):
            while (True):
                try:
                    request = self.requests.get(0.1)
                except:
                    continue
                if self.exit:
                    return
                if request == None:
                    continue
                if request == 'refreshApplications':
                    self.emit( SIGNAL('refreshApplications()'))
                elif request[0] == 'releaseApplication(QString)':
                    self.emit( SIGNAL('releaseApplication(QString)'), request[1])
                elif request[0] == 'shutdownDeviceManager(QString)':
                    self.emit( SIGNAL('shutdownDeviceManager(QString)'), request[1])
                elif request == 'refreshDeviceManagers':
                    self.emit( SIGNAL('refreshDeviceManagers()'))
                elif request == 'parseDomainManager':
                    self.emit( SIGNAL('parseDomainManager()'))
                elif request == 'parseApplications':
                    self.emit( SIGNAL('parseApplications()'))
                elif request == 'parseAllDeviceManagers':
                    self.emit( SIGNAL('parseAllDeviceManagers()'))
                elif request == 'setupDomain':
                    self.emit( SIGNAL('setupDomain()'))
                elif request == 'cleardomMgrItem':
                    self.emit( SIGNAL('cleardomMgrItem()'))
                elif request == 'cleardevMgrItem':
                    self.emit( SIGNAL('cleardevMgrItem()'))
                elif request == 'clearappsItem':
                    self.emit( SIGNAL('clearappsItem()'))
                elif request[0] == 'setWindowTitle':
                    self.emit( SIGNAL('setWindowTitle(QString)'), request[1] )
                else:
                    print request,"...unrecognized"
            return

    def addRequest(self, request):
        self._requests.put(request)

    def removeWidget(self, widget):
        try:
            widget[0].disconnect(widget[1])
        except:
            pass
        if hasattr(widget[1], 'releaseObject'):
            widget[1].releaseObject()
        if hasattr(widget[1], 'close'):
            widget[1].close()

    def applicationODMEvent(self, event):
        self._requests.put('refreshApplications')

    def unusedODMEvent(self, event):
        pass

    def devMgrODMEvent(self, event):
        self._requests.put('refreshDeviceManagers')

    def cleanupOnExit(self):
        self.removeAllWidgets()
        self.disconnectFromODMChannel()
        self.worker.end()

    def removeAllWidgets(self):
        for app in self.apps:
            for widget in app['widgets']:
                self.removeWidget(widget)

    def connectToODMChannel(self):
        self.odmListener = ODMListener()
        self.odmListener.connect(self.domManager)
        self.odmListener.deviceManagerAdded.addListener(WeakBoundMethod(self.devMgrODMEvent))
        self.odmListener.deviceManagerRemoved.addListener(WeakBoundMethod(self.devMgrODMEvent))
        self.odmListener.applicationAdded.addListener(WeakBoundMethod(self.applicationODMEvent))
        self.odmListener.applicationRemoved.addListener(WeakBoundMethod(self.applicationODMEvent))
        self.odmListener.applicationFactoryAdded.addListener(WeakBoundMethod(self.unusedODMEvent))
        self.odmListener.applicationFactoryRemoved.addListener(WeakBoundMethod(self.unusedODMEvent))
        self.odmListener.deviceAdded.addListener(WeakBoundMethod(self.devMgrODMEvent))
        self.odmListener.deviceRemoved.addListener(WeakBoundMethod(self.devMgrODMEvent))
        self.odmListener.serviceAdded.addListener(WeakBoundMethod(self.devMgrODMEvent))
        self.odmListener.serviceRemoved.addListener(WeakBoundMethod(self.devMgrODMEvent))

    def disconnectFromODMChannel(self):
        if self.odmListener != None:
            try:
                self.odmListener.disconnect()
            except:
                self.log.trace("unable to disconnect from ODM channel")
        
    def findDomains(self):
        domainList = redhawk.scan()
        if len(domainList) == 1:
            return domainList[0]
        stringlist = QStringList()
        for entry in domainList:
            stringlist.append(QString(entry))
        retval = QInputDialog.getItem(None, QString("pick a domain"), QString("domains"), stringlist, 0, False)
        if retval[1] == False:
            return None
        return str(retval[0])
        
    def getDomain(self, domainName):
        try:
            self.currentDomain = domainName
            self.disconnectFromODMChannel()
            self.domManager = redhawk.attach(domainName)
            self.connectToODMChannel()

            if self.domManager is None:
                raise StandardError('Could not narrow Domain Manager')

            # this is here just to make sure that the pointer is accessible
            if self.domManager._non_existent():
                raise StandardError("Unable to access the Domain Manager in naming context "+domainName)
        except:
            raise StandardError("Unable to access the Domain Manager in naming context "+domainName)

        try:
            self.fileMgr = self.domManager._get_fileMgr()
        except:
            raise StandardError("Unable to access the File Manager in Domain Manager "+domainName)

    def updateDomain(self, domainName):

        self.getDomain(domainName)
        self.setupDomain()

        self._requests.put('parseDomainManager')
        self._requests.put('parseApplications')
        self._requests.put('parseAllDeviceManagers')

    def cleardomMgrItem(self):
        clearChildren(self.domMgrItem)

    def cleardevMgrItem(self):
        clearChildren(self.devMgrItem)

    def clearappsItem(self):
        clearChildren(self.appsItem)

    def refreshView (self):
        if self.currentDomain == "":
            domainName = self.findDomains()
            if not domainName:
                return
        else:
            domainName = self.currentDomain

        self.getDomain(domainName)
        self._requests.put('cleardomMgrItem')
        self._requests.put('cleardevMgrItem')
        self._requests.put('clearappsItem')
        self._requests.put('parseDomainManager')
        self._requests.put('parseApplications')
        self._requests.put('parseAllDeviceManagers')

        if domainName == None:
            self._requests.put(('setWindowTitle', "REDHAWK Domain Browser"))
        else:
            self._requests.put(('setWindowTitle', "REDHAWK Domain Browser: "+domainName))

    def getAttribute (self, item, attribute):
        child = item.firstChild()
        while child:
            if child.text(0) == attribute:
                return str(child.text(1))
            child = child.nextSibling()
        return None

    def findApplication (self, id, name):
        for app in self.domManager._get_applications():
            if app._get_identifier() == id:
                return app
        return None


    def createSelected (self):
        appList = self.domManager.catalogSads()

        # Present the app list in alphabetical order.
        appList.sort()
        app = installdialog.getApplicationFile(appList, self)
        if app == None:
            return
        try:
            app_inst = self.domManager.createApplication(app)
        except CF.ApplicationFactory.CreateApplicationError, e:
            QMessageBox.critical(self, 'Creation of waveform failed.', e.msg, QMessageBox.Ok)
            return
        if app_inst == None:
            QMessageBox.critical(self, 'Creation of waveform failed.', 'Unable to create Application instance for $SDRROOT'+app, QMessageBox.Ok)

        self._requests.put('clearappsItem')
        self._requests.put('parseApplications')

    def propertyChanged (self, item):
        # First, make sure a property changed, then find the owning application
        # (note the this wouldn't work for device properties).
        parent = item.parent()
        if not parent or parent.text(0) != 'Properties':
            return
        parent = parent.parent()
        if not parent:
            return

        # NB: Using the hidden property id to set the property; may want a better
        #     lookup method.
        id = str(item.text(3))
        value = str(item.text(1))
        try:
            value = eval(value)
        except:
            pass
        
        if parent.text(0) == 'Domain Manager':
            propset = self.domManager
            value = toAny(value, self.domMgrProps[id]['type'])
        else:
            element = self.getAttribute(parent, 'Naming Element')
            if not element:
                return
            propset = lookupPropertySet(self.rootContext, element)
            value = any.to_any(value)
        
        try:
            propset.configure([CF.DataType(id, value)])
        except:
            pass

    ####################################
    # These calls interact with the UI
    # Access them only from the QThread
    def setupDomain(self):
        self.domainLabel.setText(self.currentDomain + ' ' + self.domManager._get_identifier())
        self.objectListView.setContextMenuPolicy(Qt.CustomContextMenu)
        self.domMgrItem = self.addTreeWidgetItem(self.objectListView, 'Domain Manager')
        self.devMgrItem = self.addTreeWidgetItem(self.objectListView, 'Device Managers')
        self.appsItem = self.addTreeWidgetItem(self.objectListView, 'Applications')
        self.objectListView.setCallbackObject(self)

    def releaseApplication(self, appname):
        foundApp = False
        for app in self.apps:
            if app['app_ref'].name == appname:
                foundApp = True
                break
        if foundApp:
            for widget in app['widgets']:
                self.removeWidget(widget)
            app['app_ref'].releaseObject()

    def shutdownDeviceManager(self, devMgrname):
        foundDevMgr = False
        for devMgr in self.devMgrs:
            if devMgr['devMgr_ref'].name == devMgrname:
                foundDevMgr = True
                break
        if foundDevMgr:
            for widget in devMgr['widgets']:
                self.removeWidget(widget)
            devMgr['devMgr_ref'].shutdown()

    def refreshApplications(self):
        self.clearappsItem()
        self.parseApplications()

    def refreshDeviceManagers(self):
        self.cleardevMgrItem()
        self.parseAllDeviceManagers()

    def parseApplications (self):
        # Build the list of installed applications.
        self.apps = []
        for app in self.domManager.apps:
            id = app._get_identifier()
            name = app.name
            profile = app._get_profile()
            appItem = self.addTreeWidgetItem(self.appsItem, name)
            self.apps.append({'application':appItem})
            appIdentifier = self.addTreeWidgetItem(appItem, 'Identifier', id)
            appProfile = self.addTreeWidgetItem(appItem, 'Profile', profile)
            appName = self.addTreeWidgetItem(appItem, 'Name', name)
            appPortItem = self.addTreeWidgetItem(appItem, 'Ports')
            for port in app.ports:
                if port._using == None:
                    portItem = self.addTreeWidgetItem(appPortItem, port._name+' (prov)', port._interface.name)
                else:
                    portItem = self.addTreeWidgetItem(appPortItem, port._name+' (uses)', port._using.name)
                

            componentItem = self.addTreeWidgetItem(appItem, 'Components')
            self.apps[-1]['components']=[]
            self.apps[-1]['app_ref']=app
            self.apps[-1]['widgets']=[]
            for comp in app.comps:
                self.apps[-1]['components'].append({'name':comp.name})
                item = self.addTreeWidgetItem(componentItem, comp.name)
                subitem = self.addTreeWidgetItem(item, 'Identifier', comp._id)
                subitem = self.addTreeWidgetItem(item, 'PID', str(comp._pid))
                portItems = self.addTreeWidgetItem(item, 'Ports')
                for port in comp.ports:
                    if port._using == None:
                        portItem = self.addTreeWidgetItem(portItems, port._name+' (prov)', port._interface.name)
                    else:
                        portItem = self.addTreeWidgetItem(portItems, port._name+' (uses)', port._using.name)
                propItem = self.addTreeWidgetItem(item, 'Properties')
                self.buildPropertiesListView(propItem, comp._properties)
                devItem = self.addTreeWidgetItem(item, 'Deployed on Devices')
                if len(comp._devs) > 0:
                    for dev in comp._devs:
                        item = self.addTreeWidgetItem(devItem, 'id', dev)

    def addTreeWidgetItem(self, parent, key, value=None):
        item = QTreeWidgetItem(parent)
        item.setText(0, key)
        if value != None:
            item.setText(1, value)
        return item

    def parseAllDeviceManagers (self):
        # Build the list of Device Managers.
        self.devMgrs = []
        for devMgr in self.domManager.devMgrs:
            self.parseDeviceManager(devMgr)

    def parseDeviceManager (self, devMgr):
        _id = devMgr.id
        label = devMgr.name
        try:
            profile = str(devMgr._get_deviceConfigurationProfile())
        except:
            print "Device Manager "+label+" unreachable"
            return

        # Create a node for the Device Manager, with its attributes and devices as children.
        dmItem = self.addTreeWidgetItem(self.devMgrItem, label)
        dmId = self.addTreeWidgetItem(dmItem, 'Identifier', _id)
        dmConf = self.addTreeWidgetItem(dmItem, 'Profile', str(devMgr._get_deviceConfigurationProfile()))
        propItem = self.addTreeWidgetItem(dmItem, 'Properties')
        deviceItem = self.addTreeWidgetItem(dmItem, 'Devices')
        serviceItem = self.addTreeWidgetItem(dmItem, 'Services')

        self.devMgrs.append({'devMgr':dmItem})
        self.devMgrs[-1]['devMgr_ref'] = devMgr
        self.devMgrs[-1]['widgets'] = []

        # Read the DCD file to get the SPD file, which can then be used to get the properties.
        _xmlFile = devMgr.fs.open(profile, True)
        dcd = minidom.parseString(_xmlFile.read(_xmlFile.sizeOf()))      
        _xmlFile.close()
        spdFile = dcd.getElementsByTagName('localfile')[0].getAttribute('name')
        if not spdFile.startswith("/"):
            spdFile = os.path.join(os.path.dirname(xmlfile), spdFile)

        # Get the property name mapping.
        prfFile = getPropertyFile(spdFile, devMgr.fs)
        devManProps = parsePropertyFile(prfFile, devMgr.fs)
        try:
            props = devMan.query([])
        except:
            props = []
        self.buildPropertiesListView_old(propItem, props, devManProps)

        # Create a node for each Device under the Device Manager.
        for service in devMgr.services:
            svcItem = self.addTreeWidgetItem(serviceItem, service._instanceName, service._refid)

        # Create a node for each Device under the Device Manager.
        for device in devMgr.devs:
            _id = device._id
            label = device.name
            devItem = self.addTreeWidgetItem(deviceItem, label)
            sub = self.addTreeWidgetItem(devItem, 'Identifier', _id)
            sub = self.addTreeWidgetItem(devItem, 'Admin State', str(device._get_adminState()))
            sub = self.addTreeWidgetItem(devItem, 'Operational State', str(device._get_operationalState()))
            sub = self.addTreeWidgetItem(devItem, 'Usage State', str(device._get_usageState()))
            sub = self.addTreeWidgetItem(devItem, 'Software Profile', device._profile)

            # Build a list of the device's properties in a sub-list
            propItem = self.addTreeWidgetItem(devItem, 'Properties')
            self.buildPropertiesListView(propItem, device._properties)

    def parseDomainManager (self):
        identifier = self.addTreeWidgetItem(self.domMgrItem, 'Identifier:', self.domManager._get_identifier())
        profile = self.addTreeWidgetItem(self.domMgrItem, 'Profile:', self.domManager._get_domainManagerProfile())
        self.domMgrPropsItem = self.addTreeWidgetItem(self.domMgrItem, 'Properties')

        # Read the DMD file to get the SPD file, which can then be used to get the properties.
        _xmlFile = self.fileMgr.open(str(self.domManager._get_domainManagerProfile()), True)
        dmd = minidom.parseString(_xmlFile.read(_xmlFile.sizeOf()))      
        _xmlFile.close()
        spdFile = dmd.getElementsByTagName('localfile')[0].getAttribute('name')
        if not spdFile.startswith("/"):
            spdFile = os.path.join(os.path.dirname(xmlfile), spdFile)

        # Get the property name mapping.
        prfFile = getPropertyFile(spdFile, self.fileMgr)
        self.domMgrProps = parsePropertyFile(prfFile, self.fileMgr)

        # Create entries for all of the properties.
        try:
            props = self.domManager.query([])
        except:
            props = []
        self.buildPropertiesListView_old(self.domMgrPropsItem, props, self.domMgrProps)

    def parseDomain (self):
        self.parseDomainManager()
        self.parseApplications()
        self.parseAllDeviceManagers()

    def buildPropertiesListView_old (self, parent, properties, propertyDefs):
        for prop in properties:
            id = prop.id
            value = any.from_any(prop.value)
            if id not in propertyDefs:
                valueItem = QListViewItem(parent, id, str(value), '', id)
                continue
            property = propertyDefs[id]
            name = property.get('name', id)
            if property['elementType'] in ('simple', 'simplesequence'):
                # NB: As a temporary hack for mapping back to property id, store the id in
                #     the hidden fourth field.
                valueItem = self.addTreeWidgetItem(self.domMgrPropsItem, name, str(value))
                #if property['mode'] in ('readwrite', 'writeonly'):
                #    valueItem.setRenameEnabled(1, True)
            elif property['elementType'] == 'struct':
                valueItem = QListViewItem(parent, name, '', '', id)
                for field in value:
                    QListViewItem(valueItem, field['id'], str(field['value']))
            elif property['elementType'] == 'structsequence':
                valueItem = QListViewItem(parent, name, '', '', id)
                if value == None:
                    continue
                for index, structval in enumerate(value):
                    subitem = QListViewItem(valueItem, str(index))
                    for field in structval:
                        QListViewItem(subitem, field['id'], str(field['value']))

    def buildPropertiesListView (self, parent, properties):
        for prop in properties:
            id = prop.id
            value = str(prop.queryValue())
            name = prop.clean_name
            if prop.__class__ == prop_helpers.simpleProperty or prop.__class__ == prop_helpers.sequenceProperty:
                valueItem = self.addTreeWidgetItem(parent, name, str(value))
                #if property['mode'] in ('readwrite', 'writeonly'):
                #    valueItem.setRenameEnabled(1, True)
            elif prop.__class__ == prop_helpers.structProperty:
                valueItem = self.addTreeWidgetItem(parent, name)
                for field in prop.members:
                    valueSubItem = self.addTreeWidgetItem(valueItem, field, str(prop.members[field].queryValue()))
            elif prop.__class__ == prop_helpers.structSequenceProperty:
                valueItem = self.addTreeWidgetItem(parent, name)
                idx_count = 0
                for entry in prop.queryValue():
                    valueSubItem = self.addTreeWidgetItem(valueItem, '['+str(idx_count)+']')
                    for field in entry:
                        valueIdxItem = self.addTreeWidgetItem(valueSubItem, field, str(entry[field]))

    def debug (self, *args):
        if not self.verbose:
            return
        for arg in args:
            print arg,
        print
