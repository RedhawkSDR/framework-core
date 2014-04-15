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


# Form implementation generated from reading ui file 'browsewindowbase.ui'
#
# Created: Thu Feb 4 17:22:26 2010
#      by: The PyQt User Interface Compiler (pyuic) 3.13
#
# WARNING! All changes made in this file will be lost!


from PyQt4.QtGui import *
from PyQt4.QtCore import *
from ossie.utils import sb
import copy

class TreeWidget(QTreeWidget):
    def __init__(self, parent=None):
        QTreeWidget.__init__(self, parent)
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self.contextMenuEvent)
        self.appObject = None
        self.callbackObject = None

    def setCallbackObject(self, obj):
        self.callbackObject = obj
        self.appObject = obj.appsItem

    def getPos(self, item, parent):
        pos = None
        if item is not None:
            parent = item.parent()
            while parent is not None:
                parent.setExpanded(True)
                parent = parent.parent()
            itemrect = self.visualItemRect(item)
            pos = self.mapToGlobal(itemrect.center())
        return pos

    def contextMenuEvent(self, event):
        pos = None
        selection = self.selectedItems()
        if selection:
            item = selection[0]
        else:
            item = self.currentItem()
            if item is None:
                item = self.invisibleRootItem().child(0)

        if item.parent() != None and item.parent().text(0) == "Components":
            appname = item.parent().parent().text(0)
            compname = item.text(0)
            foundApp = False
            for app in self.callbackObject.apps:
                if app['app_ref'].name == appname:
                    appref = app['app_ref']
                    appTrack = app
                    foundApp = True
                    break
            if not foundApp:
                self.callbackObject.log.warn("Unable to find application: "+appname)
                return
            foundComp = False
            for comp in appref.comps:
                if comp.name == compname:
                    foundComp = True
                    break
            if not foundComp:
                self.callbackObject.log.warn("Unable to find component ("+compname+") on application ("+appname+")")
            pos = self.getPos(item, item.parent())
            if pos is not None:
                menu = QMenu(self)
                startAction = menu.addAction('Start')
                if comp._get_started():
                    startAction.setEnabled(False)
                stopAction = menu.addAction('Stop')
                if not comp._get_started():
                    stopAction.setEnabled(False)
                foundBulkio = False
                for port in comp.ports:
                    if port._using != None:
                        if port._using.nameSpace == 'BULKIO':
                            foundBulkio = True
                            break
                if foundBulkio:
                    plotAction = menu.addAction('Plot')
                    soundAction = menu.addAction('Sound')
                menu.popup(pos)
                retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Start':
                    comp.start()
                elif resp == 'Stop':
                    comp.stop()
                elif resp == 'Plot':
                    plot=sb.LinePlot()
                    plot.start()
                    try:
                        comp.connect(plot)
                        appTrack['widgets'].append((comp, plot))
                    except Exception, e:
                        plot.close()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        comp.connect(sound)
                        appTrack['widgets'].append((comp, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
        elif item.parent() == self.appObject:
            appname = item.text(0)
            appref = None
            appTrack = None
            foundApp = False
            for app in self.callbackObject.apps:
                if app['app_ref'].name == appname:
                    appref = app['app_ref']
                    appTrack = app
                    foundApp = True
                    break
            if not foundApp:
                self.callbackObject.log.warn("Unable to find application: "+appname)
                return
            pos = self.getPos(item, item.parent())
            retval = None
            if pos is not None:
                menu = QMenu(self)
                relAction = menu.addAction('Release')
                startAction = menu.addAction('Start')
                if appref._get_started():
                    startAction.setEnabled(False)
                stopAction = menu.addAction('Stop')
                if not appref._get_started():
                    stopAction.setEnabled(False)
                foundBulkio = False
                for port in appref.ports:
                    if port._using != None:
                        if port._using.nameSpace == 'BULKIO':
                            foundBulkio = True
                            break
                if foundBulkio:
                    plotAction = menu.addAction('Plot')
                    soundAction = menu.addAction('Sound')
                menu.popup(pos)
                retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Release':
                    self.callbackObject.releaseApplication(appname)
                elif resp == 'Start':
                    appref.start()
                elif resp == 'Stop':
                    appref.stop()
                elif resp == 'Plot':
                    plot=sb.LinePlot()
                    plot.start()
                    try:
                        appref.connect(plot)
                        appTrack['widgets'].append((appref, plot))
                    except Exception, e:
                        plot.close()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific Port in the Application Port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        appref.connect(sound)
                        appTrack['widgets'].append((appref, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
        elif item.parent() != None and item.parent().text(0) == "Ports" and item.parent().parent() != None and item.parent().parent().parent().text(0) == "Components":
            compname = item.parent().parent().text(0)
            appname = item.parent().parent().parent().parent().text(0)
            portname = item.text(0)[:-7]
            appref = None
            appTrack = None
            foundApp = False
            for app in self.callbackObject.apps:
                if app['app_ref'].name == appname:
                    appref = app['app_ref']
                    appTrack = app
                    foundApp = True
                    break
            if not foundApp:
                self.callbackObject.log.warn("Unable to find application: "+appname)
                return
            foundComp = False
            for comp in appref.comps:
                if comp.name == compname:
                    foundComp = True
                    break
            if not foundComp:
                self.callbackObject.log.warn("Unable to find component "+compname+" on application "+appname)
                return
            foundPort = False
            for port in comp.ports:
                if port._name == portname:
                    foundPort = True
                    break
            if not foundPort:
                self.callbackObject.log.warn("Unable to find port "+portname+" in component "+compname+" on application "+appname)
                return
            pos = self.getPos(item, item.parent())
            retval = None
            if pos is not None:
                foundBulkio = False
                if port._using != None:
                    if port._using.nameSpace == 'BULKIO':
                        foundBulkio = True
                if foundBulkio:
                    menu = QMenu(self)
                    plotAction = menu.addAction('Plot')
                    soundAction = menu.addAction('Sound')
                    menu.popup(pos)
                    retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Plot':
                    plot=sb.LinePlot()
                    plot.start()
                    try:
                        if port._using != None:
                            comp.connect(plot,usesPortName=str(portname))
                        appTrack['widgets'].append((appref, plot))
                    except Exception, e:
                        plot.close()
                        QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        comp.connect(sound)
                        appTrack['widgets'].append((comp, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
        elif item.parent() != None and item.parent().text(0) == "Ports" and item.parent().parent() != None and item.parent().parent().parent() == self.appObject:
            appname = item.parent().parent().text(0)
            portname = item.text(0)[:-7]
            appref = None
            appTrack = None
            foundApp = False
            for app in self.callbackObject.apps:
                if app['app_ref'].name == appname:
                    appref = app['app_ref']
                    appTrack = app
                    foundApp = True
                    break
            if not foundApp:
                self.callbackObject.log.warn("Unable to find application: "+appname)
                return
            foundPort = False
            for port in appref.ports:
                if port._name == portname:
                    foundPort = True
                    break
            if not foundPort:
                self.callbackObject.log.warn("Unable to find port "+portname+" on application "+appname)
                return
            pos = self.getPos(item, item.parent())
            retval = None
            if pos is not None:
                foundBulkio = False
                if port._using != None:
                    if port._using.nameSpace == 'BULKIO':
                        foundBulkio = True
                if foundBulkio:
                    menu = QMenu(self)
                    plotAction = menu.addAction('Plot')
                    soundAction = menu.addAction('Sound')
                    menu.popup(pos)
                    retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Plot':
                    plot=sb.LinePlot()
                    plot.start()
                    try:
                        if port._using != None:
                            appref.connect(plot,usesPortName=str(portname))
                        appTrack['widgets'].append((appref, plot))
                    except Exception, e:
                        plot.close()
                        QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        appref.connect(sound)
                        appTrack['widgets'].append((appref, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)

        elif item == self.appObject:
            itemrect = self.visualItemRect(item)
            pos = self.mapToGlobal(itemrect.center())
            menu = QMenu(self)
            menu.addAction('Create Application')
            menu.addAction('Release All Applications')
            menu.popup(pos)
            retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Create Application':
                    self.callbackObject.createSelected()
                elif resp == 'Release All Applications':
                    applist = []
                    for app in self.callbackObject.apps:
                        applist.append(app['app_ref'].name)
                    for appname in applist:
                        self.callbackObject.addRequest(('releaseApplication(QString)',appname))

class BrowseWindowBase(QMainWindow):
    def __init__(self,parent = None,name = None,fl = 0,domainName=None):
        QMainWindow.__init__(self)
        self.statusBar()

        self.setCentralWidget(QWidget(self))
        BrowseWindowBaseLayout = QVBoxLayout(self.centralWidget())
        windowWidth = 720
        windowHeight = 569

        layout1 = QHBoxLayout(None)

        self.textLabel1 = QLabel("textLabel1", self.centralWidget())
        layout1.addWidget(self.textLabel1)

        self.domainLabel = QLabel("domainLabel", self.centralWidget())
        layout1.addWidget(self.domainLabel)
        spacer1 = QSpacerItem(361,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout1.addItem(spacer1)
        BrowseWindowBaseLayout.addLayout(layout1)

        self.objectListView = TreeWidget(self.centralWidget())
        self.objectListView.setColumnCount(2)
        self.objectListView.setHeaderLabels(['item', 'value'])
        self.objectListView.header().resizeSection(0, windowWidth/2-20)
        BrowseWindowBaseLayout.addWidget(self.objectListView)

        layout12 = QGridLayout(None)
        self.refreshButton = QPushButton(self.centralWidget())
        layout12.addWidget(self.refreshButton,0,0)
        BrowseWindowBaseLayout.addLayout(layout12)

        self.languageChange(domainName)

        self.resize(QSize(windowWidth,windowHeight).expandedTo(self.minimumSizeHint()))
        self.connect(self.refreshButton,SIGNAL("clicked()"),self.refreshView)
        self.connect(self.objectListView,SIGNAL("itemRenamed(QListViewItem*,int)"),self.propertyChanged)

    def closeEvent(self, event):
        self.cleanupOnExit()
        event.accept()

    def languageChange(self, domainName):
        if domainName == None:
            self.setWindowTitle(self.__tr("REDHAWK Domain Browser"))
        else:
            self.setWindowTitle(self.__tr("REDHAWK Domain Browser: "+domainName))
        self.textLabel1.setText(self.__tr("Domain:"))
        self.domainLabel.setText(QString())
        self.refreshButton.setText(self.__tr("&Refresh"))

    def refreshView(self):
        print "BrowseWindowBase.refreshView(): Not implemented yet"

    def propertyChanged(self):
        print "BrowseWindowBase.propertyChanged(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("BrowseWindowBase",s,c)
