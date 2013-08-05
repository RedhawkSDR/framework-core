import code
from code import SourceFile

class CppFactory(code.Factory):
    def __init__ (self):
        classes = (Code, Scope, If, Try, Class, Function, Namespace)
        super(CppFactory, self).__init__(*classes)

Code = code.factoryclass(code.Code, CppFactory)
Scope = code.factoryclass(code.Scope, CppFactory)
If = code.factoryclass(code.If, CppFactory)
Try = code.factoryclass(code.Try, CppFactory)
Function = code.factoryclass(code.Function, CppFactory)

class Class(Scope):
    def __init__ (self, name, inherits=[]):
        super(Class, self).__init__('{', '};')
        self.__name = name
        self.__parents = inherits

    def write (self, f):
        decl = 'class ' + self.__name
        if self.__parents:
            decl += ' : ' + ', '.join(self.__parents)
        f.write(decl)
        super(Class, self).write(f)

class Namespace(Scope):
    def __init__ (self, name):
        super(Namespace, self).__init__('namespace '+name+' {')
        self.__name = name

class Module(Code):
    def __init__ (self):
        super(Module, self).__init__()
        self.__includes = []

    def include (self, hfile):
        if hfile in self.__includes:
            return
        self.__includes.append(hfile)
        
    def write (self, f):
        for hfile in self.__includes:
            f.write('#include %s', hfile)
        f.write()

        super(Module, self).write(f)

class Header(Module):
    def __init__ (self, name):
        super(Header, self).__init__()
        self.__guard = '__%s__' % (name.upper().replace('.', '_'))

    def write(self, f):
        f.write('#ifndef %s', self.__guard)
        f.write('#define %s', self.__guard)
        f.write()

        super(Header, self).write(f)

        f.write()
        f.write('#endif // %s', self.__guard)
