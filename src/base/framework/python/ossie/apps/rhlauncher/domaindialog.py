from PyQt4 import QtGui

import ui

class DomainDialog(QtGui.QDialog):
    def __init__(self, *args, **kwargs):
        super(DomainDialog,self).__init__(*args, **kwargs)
        ui.load('domaindialog.ui', self)

    def persistence(self):
        return self.persistCheckBox.isChecked()

    def setPersistence(self, enabled):
        self.persistCheckBox.setChecked(enabled)

    def domainName(self):
        return str(self.domainNameEdit.text())

    def setDomainName(self, name):
        self.domainNameEdit.setText(name)

    def debugLevel(self):
        return self.logLevelComboBox.currentIndex()
