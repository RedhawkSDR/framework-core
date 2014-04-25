import os

import PyQt4.uic

def load(filename, widget):
    oldpath = os.getcwd()
    os.chdir(os.path.dirname(__file__))
    try:
        PyQt4.uic.loadUi(filename, widget)
    finally:
        os.chdir(oldpath)
