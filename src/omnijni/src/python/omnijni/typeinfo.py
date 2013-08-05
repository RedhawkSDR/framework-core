from omniidl import idltype, idlast

_primitiveTypes = (
    idltype.tk_void,
    idltype.tk_boolean,
    idltype.tk_octet,
    idltype.tk_char,
    idltype.tk_short,
    idltype.tk_ushort,
    idltype.tk_long,
    idltype.tk_ulong,
    idltype.tk_longlong,
    idltype.tk_ulonglong,
    idltype.tk_float,
    idltype.tk_double,
)

def isEnum (itype):
    if not isinstance(itype, idltype.Type):
        return False
    itype = itype.unalias()
    return isinstance(itype, idltype.Declared) and isinstance(itype.decl(), idlast.Enum)

def isPrimitiveType (itype):
    if not isinstance(itype, idltype.Type):
        return False
    return itype.unalias().kind() in _primitiveTypes

def isString (itype):
    if not isinstance(itype, idltype.Type):
        return False
    return isinstance(itype.unalias(), idltype.String)

def isSequence (itype):
    if not isinstance(itype, idltype.Type):
        return False
    return isinstance(itype.unalias(), idltype.Sequence)

def isStruct (itype):
    itype = itype.unalias()
    return isinstance(itype, idltype.Declared) and isinstance(itype.decl(), idlast.Struct)

def isInterface (itype):
    itype = itype.unalias()
    if isinstance(itype, idltype.Declared):
        return isinstance(itype.decl(), (idlast.Interface, idlast.Forward))
    return False

def isDeclared (itype):
    return isinstance(itype.unalias(), idltype.Declared)

def isAny (itype):
    if not isinstance(itype, idltype.Type):
        return False
    itype = itype.unalias()
    return isinstance(itype, idltype.Base) and itype.kind() == idltype.tk_any

def isBuiltIn (itype):
    if isAny(itype):
        return True
    elif not isDeclared(itype):
        return False
    return itype.unalias().decl().builtIn()

class Method:

    def __init__ (self, name, returnType, parameters, exceptions=()):
        self.__name = name
        self.__returnType = returnType
        self.__parameters = parameters
        self.__exceptions = exceptions

    def name (self):
        return self.__name

    def stubName (self):
        return self.__name

    def returnType (self):
        return self.__returnType

    def parameters (self):
        return self.__parameters

    def exceptions (self):
        return self.__exceptions

class Getter (Method):

    def __init__ (self, name, attrType):
        Method.__init__(self, name, attrType, [], [])

    def stubName (self):
        return '_get_' + self.name()

class Setter (Method):
    
    def __init__ (self, name, attrType):
        returnType = idltype.Base(idltype.tk_void)
        param = idlast.Parameter(None, None, None, [], [], 0, attrType, "value")
        Method.__init__(self, name, returnType, [param], [])

    def stubName (self):
        return '_set_' + self.name()


class MethodVisitor:

    def getMethods (self, node):
        self.__methods = []
        node.accept(self)
        return self.__methods
    
    def visitInterface (self, node):
        for call in node.all_callables():
            call.accept(self)

    def visitAttribute (self, node):
        name = node.identifiers()[0]
        self.__methods.append(Getter(name, node.attrType()))
        if not node.readonly():
            self.__methods.append(Setter(name, node.attrType()))
    
    def visitOperation (self, node):
        self.__methods.append(Method(node.identifier(), node.returnType(), node.parameters(), node.raises()))
