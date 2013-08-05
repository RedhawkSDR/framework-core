import commands

class Terminal(object):
    def __init__(self, command):
        status, self.__command = commands.getstatusoutput('which '+command)
        if status:
            raise RuntimeError, command + ' cannot be found'

    def _termOpts(self):
        return []

    def command(self, command, arguments, title=None):
        options = self._termOpts()
        if title:
            options += self._titleArgs(title)
        return self.__command, options + self._execArgs(command, arguments)

class XTerm(Terminal):
    def __init__(self):
        super(XTerm,self).__init__('xterm')

    def _titleArgs(self, title):
        return ['-T', title]

    def _execArgs(self, command, arguments):
        return ['-e', command] + arguments

class GnomeTerm(Terminal):
    def __init__(self):
        super(GnomeTerm,self).__init__('gnome-terminal')

    def _titleArgs(self, title):
        return ['-t', title]

    def _execArgs(self, command, arguments):
        return ['-x', command] + arguments
