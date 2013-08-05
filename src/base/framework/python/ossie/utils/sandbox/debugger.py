import commands
import os
import logging
import sys

class Debugger(object):
    def __init__(self, command):
        self.command = command

    def isInteractive(self):
        return True

    def modifiesCommand(self):
        return False

    def canAttach(self):
        return False

class GDB(Debugger):
    def __init__(self):
        status, gdb = commands.getstatusoutput('which gdb')
        if status:
            raise RuntimeError, 'gdb cannot be found'
        super(GDB,self).__init__(gdb)

    def canAttach(self):
        return True

    def attach(self, process):
        return self.command, [process.command(), str(process.pid())]

class PDB(Debugger):
    def __init__(self):
        super(PDB,self).__init__(PDB.findPDB())

    def modifiesCommand(self):
        return True

    def wrap(self, command, arguments):
        return self.command, [command] + arguments

    @staticmethod
    def findPDB():
        for path in sys.path:
            filename = os.path.join(path, 'pdb.py')
            if os.path.isfile(filename):
                return filename
        raise RuntimeError, 'pdb cannot be found'


class Valgrind(Debugger):
    def __init__(self, quiet=False, verbose=False, **opts):
        status, valgrind = commands.getstatusoutput('which valgrind')
        if status:
            raise RuntimeError, 'valgrind cannot be found'
        super(Valgrind,self).__init__(valgrind)
        self.arguments = []
        if quiet:
            self.arguments.append('-q')
        if verbose:
            self.arguments.append('-v')
        for name, value in opts.iteritems():
            optname = '--' + name.replace('_','-')
            if value is True:
                value = 'yes'
            elif value is False:
                value = 'no'
            self.arguments.append(optname + '=' + str(value))

    def modifiesCommand(self):
        return True

    def isInteractive(self):
        return False

    def wrap(self, command, arguments):
        return self.command, self.arguments + [command] + arguments

