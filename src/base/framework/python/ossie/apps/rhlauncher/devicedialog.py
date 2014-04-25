import os
import glob

from PyQt4 import QtGui

from ossie.parsers import dcd
from ossie.utils import redhawk

import ui

class DeviceDialog(QtGui.QDialog):
    def __init__(self, *args, **kwargs):
        super(DeviceDialog,self).__init__(*args, **kwargs)
        ui.load('devicedialog.ui', self)
        self._nodes = {}
        for domain in redhawk.scan():
            self.domainNameEdit.addItem(domain)

    def setSdrRoot(self, sdrroot):
        self.nodeListWidget.clear()
        nodepath = os.path.join(sdrroot, 'dev/nodes/*/DeviceManager.dcd.xml')
        for dcdfile in glob.glob(nodepath):
            try:
                node = dcd.parse(dcdfile)
                name = node.get_name()
                domain = node.get_domainmanager().get_namingservice().get_name()
                domain = domain.split('/')[-1]
                self._nodes[name] = {'dcd':dcdfile, 'domain':domain}
            except:
                pass
        for name in self._nodes.keys():
            QtGui.QListWidgetItem(name, self.nodeListWidget)

    def selectionChanged(self, index):
        name = str(self.nodeListWidget.item(index).text())
        self.setDomainName(self._nodes[name]['domain'])

    def domainName(self):
        return str(self.domainNameEdit.currentText())

    def setDomainName(self, name):
        index = self.domainNameEdit.findText(name)
        if index >= 0:
            self.domainNameEdit.setCurrentIndex(index)

    def debugLevel(self):
        return self.logLevelComboBox.currentIndex()

    def nodeName(self):
        return str(self.nodeListWidget.currentItem().text())

    def dcdFile(self):
        return self._nodes[self.nodeName()]['dcd']
