import os
import sys

from PyQt4 import QtGui

from launcherwindow import LauncherWindow

def main():
    sdrroot = os.environ.get('SDRROOT', None)
    if not sdrroot:
        raise SystemExit('SDRROOT must be set')

    app = QtGui.QApplication(sys.argv)
    mainwindow = LauncherWindow(sdrroot)
    mainwindow.show()
    sys.exit(app.exec_())
