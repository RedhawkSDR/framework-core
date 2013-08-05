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

from ossie.properties import _SCA_TYPES
from ossie.properties import __TYPE_MAP
from ossie.properties import getPyType 
from ossie.properties import getTypeCode
from ossie.properties import getCFType 
from ossie.properties import getMemberType 
from ossie.properties import getCFSeqType 
from ossie import parsers as _parsers
from ossie.utils import type_helpers as _type_helpers
from ossie.cf import CF as _CF
from ossie.cf import PortTypes as _PortTypes
from omniORB import any as _any
from omniORB import CORBA as _CORBA
from omniORB import tcInternal as _tcInternal
import copy as _copy
import struct as _struct
import string as _string
import operator as _operator
from ossie.utils.type_helpers import OutOfRangeException

SCA_TYPES = globals()['_SCA_TYPES']

# Map the type of the complex number (e.g., complexFloat) to the 
# type of the real and imaginary members (e.g., float).
__COMPLEX_SIMPLE_TYPE_MAP = {
    'complexFloat'    : 'float',
    'complexBoolean'  : 'boolean',
    'complexULong'    : 'ulong',
    'complexShort'    : 'short',
    'complexOctet'    : 'octet',
    'complexChar'     : 'char',
    'complexUShort'   : 'ushort',
    'complexDouble'   : 'double',
    'complexLong'     : 'long',
    'complexLongLong' : 'longlong',
    'complexULongLong': 'ulonglong'
}

def mapComplexToSimple(complexType):
    return __COMPLEX_SIMPLE_TYPE_MAP[complexType]
    

#helper function for configuring simple and sequence properties
def configureProp(compRef, propName, propValue):
    propRef = _CF.DataType(id=str(str(propName)), value=_any.to_any(None))
    
    #try the query to see if we can get the valid type
    results = compRef.query([propRef])
    if results != None:
        if results[0].value._t != _CORBA.TC_null:
            propRef.value._t = esults[0].value._t
            propRef.value._v = propValue
            compRef.configure([propRef])
            return
    
    #if we couldn't find the type of the property, then we need
    #to try ones that might fit until one works
    applicableTypes = None
    if type(propValue) == list:
        strTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.StringSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.CharSeq)]
        boolTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.BooleanSeq)]
        longTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.ULongSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ShortSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.OctetSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.UShortSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.LongSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.LongLongSequence),
                       _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.UlongLongSequence)]
        floatTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.FloatSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.DoubleSeq)]
        
        if len(propValue) > 0:
            valueType = type(propValue[0])
        else:
            applicableTypes = strListTypes + boolListTypes + longListTypes + floatListTypes
    else:
        strTypes   = [getTypeCode('string'), 
                      getTypeCode('char')]
        boolTypes  = [getTypeCode('boolean')]
        longTypes  = [getTypeCode('ulong'), 
                      getTypeCode('short'), 
                      getTypeCode('octet'), 
                      getTypeCode('ushort'),
                      getTypeCode('long'), 
                      getTypeCode('longlong'), 
                      getTypeCode('ulonglong')]
        floatTypes = [getTypeCode('float'), 
                      getTypeCode('double')]
        
        valueType = type(propValue)
        
    if not applicableTypes:
        if valueType == str:
            applicableTypes = strTypes
        elif valueType == bool:
            applicableTypes = boolTypes
        elif valueType == long:
            applicableTypes = longTypes
        elif valueType == float:
            applicableTypes = floatTypes
        else:
            raise Exception, 'Could not match "'+str(valueType)+'" to a valid CORBA type'
    
    passConfigure = False
    for propType in applicableTypes:
        propRef.value._t = propType
        propRef.value._v = propValue
        try:
            compRef.configure([propRef])
            passConfigure = True
            break
        except:
            pass
    
    if not passConfigure:
        msg = 'Was not able to configure property: "'+str(propName)+'", trying the following types:\n'
        for propType in applicableTypes:
            msg += ('  ' + str(propType) + '\n')
        raise Exception, msg

def getPropNameDict(prf):
    #
    # make a dictionary of all of the IDs indexed by name
    #
    nameDict = {}

    # do it for all 'simple' and 'simplesequence' properties
    for prop in prf.get_simple() + prf.get_simplesequence():
        name = prop.get_name()
        if prop.get_name() == None:
            name = prop.get_id()
        else:
            name = prop.get_name()
        if nameDict.has_key( str(name)):
            print "WARN: property with non-unique name %s" % name
            continue
      
        nameDict[str(name)] = str(prop.get_id())

    # do it for all 'struct' properties
    for struct in prf.get_struct():
        if struct.get_name() == None:
            tagbase = str(struct.get_id())
        else:
            tagbase = str(struct.get_name())
        if nameDict.has_key( tagbase ):
            print "WARN: struct with duplicate name %s" % tagbase
            continue
      
        nameDict[str(tagbase)] = str(struct.get_id())
        tagbase = tagbase + "."
        for simple in struct.get_simple():
            if simple.get_name() == None:
                name = simple.get_id()
            else:
                name = simple.get_name()
            if nameDict.has_key( tagbase + str(name) ):
                print "WARN: struct element with duplicate name %s" % tagbase
                continue

            nameDict[str(tagbase + str(name))] = str(simple.get_id())

    # do it for all 'struct' sequences
    tagbase = ""
    for structSequence in prf.get_structsequence():
        if structSequence.get_name() == None:
            tagbase = str(structSequence.get_id())
        else:
            tagbase = str(structSequence.get_name())
        # make sure this structSequence name is unique
        if nameDict.has_key( str(tagbase) ):
            print "WARN: property with non-unique name %s" % tagbase
            continue

        nameDict[str(tagbase)] = str(structSequence.get_id())
        tagbase = tagbase + "."
        struct = structSequence.get_struct()

        # pull out all of the elements of the struct
        for simple in struct.get_simple():
            # make sure this struct simple element's name is unique
            if simple.get_name() == None:
                name = simple.get_id()
            else:
                name = simple.get_name()
            if nameDict.has_key( tagbase + str(name)):
                print "WARN: property with non-unique name %s" % name
                continue

            nameDict[str(tagbase + str(name))] = str(simple.get_id())
          
    return nameDict


_enums = {}
_displayNames = {}
_duplicateNames = {}

'''
-Maps a properties clean, display and access name to its ID
-Prevents duplicate entries within a component
-Allows for get/set on components with invalid chars in ID
'''
def addCleanName(cleanName, id, compRefId):
    if not _displayNames.has_key(compRefId):
        _displayNames[compRefId] = {}
        _duplicateNames[compRefId] = {}
        
    if not _displayNames[compRefId].has_key(cleanName):
        _displayNames[compRefId][cleanName] = id
        _duplicateNames[compRefId][cleanName] = 0
        return cleanName
    elif _displayNames[compRefId][cleanName] == id:
        return cleanName
    else:
        count = _duplicateNames[compRefId][cleanName] + 1
        _displayNames[compRefId][cleanName + str(count)] = id
        _duplicateNames[compRefId][cleanName] = count
        return cleanName + str(count)
    
'''
-Adds a property to the enumerated properties map
-This allows enforcement of enumerations as the property
values are configured
'''
def _addEnumerations(prop, clean_id):
    _enums[clean_id] = _parseEnumerations(prop)

def _parseEnumerations(prop):
    enums = {}
    propType = prop.get_type()
    for en in prop.get_enumerations().get_enumeration():
        if str(en.get_value()) == str(None):
            value = None
        elif propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
            if en.get_value().find('x') != -1:
                value = int(en.get_value(),16)
            else:
                value = int(en.get_value())
        elif propType in ['double', 'float']:
            value = float(en.get_value())
        elif propType in ['char', 'string']:
            value = str(en.get_value())
        elif propType == 'boolean':
            value = {"TRUE": True, "FALSE": False}[en.get_value().strip().upper()]
        else:
            value = None
        enums[str(en.get_label())] = value
    return enums


def _cleanId(prop):
    translation = 48*"_"+_string.digits+7*"_"+_string.ascii_uppercase+6*"_"+_string.ascii_lowercase+133*"_"
    prop_id = prop.get_name()
    if prop_id is None:
        prop_id = prop.get_id()
    return str(prop_id).translate(translation)

def isMatch(prop, modes, kinds, actions):
    """
    Tests whether a given SCA property (as an XML node) matches the given modes,
    kinds and actions.
    """
    if prop.get_mode() == None:
        m = "readwrite"
    else:
        m = prop.get_mode()
    matchMode = (m in modes)

    if isinstance(prop, (_parsers.PRFParser.simple, _parsers.PRFParser.simpleSequence)):
        if prop.get_action() == None:
            a = "external"
        else:
            a = prop.get_action().get_type()
        matchAction = (a in actions)

        matchKind = False
        if prop.get_kind() == None:
            k = ["configure"]
        else:
            k = prop.get_kind()
        for kind in k:
            if kind.get_kindtype() in kinds:
                matchKind = True

        # If kind is both configure and allocation, then action is a match
        # Bug 295
        foundConf = False
        foundAlloc = False
        for kind in k:
            if "configure" == kind.get_kindtype():
                foundConf = True
            if "allocation" == kind.get_kindtype():
                foundAlloc = True
        if foundConf and foundAlloc:
            matchAction = True

    elif isinstance(prop, (_parsers.PRFParser.struct, _parsers.PRFParser.structSequence)):
        matchAction = True # There is no action, so always match

        matchKind = False
        if prop.get_configurationkind() == None:
            k = ["configure"]
        else:
            k = prop.get_configurationkind()
        for kind in k:
            if kind.get_kindtype() in kinds:
                matchKind = True

        if k in kinds:
            matchKind = True


    return matchMode and matchKind and matchAction


'''
THE CLASSES BELOW SERVE AS WRAPPER CLASSES TO BE USED EXTENSIVELY IN
THE SANDBOX BUT CAN BE USED FOR POTENTIAL UNIT TESTING SCHEMES AS WELL
'''
class Property(object):
    """
    This class will act as a wrapper for a single property, and provide some
    helper type functionality to ease the use of properties
    """
    MODES = ['readwrite', 'writeonly', 'readonly']
    
    def __init__(self, id, type, compRef, mode='readwrite', action='external', parent=None):
        """ 
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        type - (string): type of property (SCA Type or 'struct' or 'structSequence')
        id - (string): the property ID
        mode - (string): mode for the property, must be in MODES
        parent - (Property): the property that contains this instance (e.g., struct that holds a simple)
        """
        self.id = id
        self.type = type
        self.compRef = compRef
        if mode not in self.MODES:
            print str(mode) + ' is not a valid mode, defaulting to "readwrite"'
            self.mode = 'readwrite'
        else:
            self.mode = mode
        self.action = action
        self._parent = parent

    def _checkRead(self):
        if self._parent:
            return self._parent._checkRead()
        return self.mode != 'writeonly'

    def _checkWrite(self):
        if self._parent:
            return self._parent._checkWrite()
        return self.mode != 'readonly'

    def _queryValue(self):
        results = self.compRef.query([_CF.DataType(str(self.id), _any.to_any(None))])
        if results is None:
            return None
        return results[0].value
            
    def _configureValue(self, value):
        self.compRef.configure([_CF.DataType(str(self.id), value)])

    def queryValue(self):
        '''
        Returns the current value of the property by doing a query
        on the component and returning only the value
        '''
        if not self._checkRead():
            raise Exception, 'Could not perform query, ' + str(self.id) + ' is a writeonly property'

        value = self._queryValue()
        return self.fromAny(value)

    def configureValue(self, value):
        '''
        Sets the current value of the property by doing a configure
        on the component. If this property is a child of another property,
        the parent property is configured.
        '''
        if not self._checkWrite():
            raise Exception, 'Could not perform configure, ' + str(self.id) + ' is a readonly property'

        try:
            value = self.toAny(value)
        except ValueError:
            # If enumeration value is invalid, list available enumerations.
            # NB: Should create specific error type for this condition.
            if self.id not in _enums:
                raise
            print 'Could not perform configure on ' + str(self.id) + ', invalid enumeration provided'
            print "Valid enumerations: "
            for enumLabel, enumValue in _enums[self.id].iteritems():
                print "\t%s=%s" % (enumLabel, enumValue)
            return
        self._configureValue(value)

    # Wrapper functions for common operations on the property value. These
    # allow the data type (e.g. int, float, list) to determine the behavior
    # of common operations, rather than having to explicitly write each
    # operator.
    def proxy_operator(op):
        """
        Performs an operator on the result of queryValue().
        """
        def wrapper(self, *args, **kwargs):
            return op(self.queryValue(), *args, **kwargs)
        return wrapper

    def proxy_reverse_operator(op):
        """
        Performs a reflected operator on the result of queryValue().
        """
        def wrapper(self, y, *args, **kwargs):
            return op(y, self.queryValue(), *args, **kwargs)
        return wrapper

    def proxy_inplace_operator(op):
        """
        Performs an in-place operator on the result of queryValue(). After
        the value is modified, the new value is stored via configureValue().
        """
        def wrapper(self, y, *args, **kwargs):
            temp = op(self.queryValue(), y, *args, **kwargs)
            self.configureValue(temp)
            return temp
        return wrapper

    def proxy_modifier_function(op):
        """
        Calls a member function that modifies the state of its instance on the
        result queryValue(). After the value is modified, the  new value is
        stored via configureValue().
        """
        def wrapper(self, *args, **kwargs):
            temp = self.queryValue()
            op(temp, *args, **kwargs)
            self.configureValue(temp)
        return wrapper

    '''
    BELOW ARE OVERRIDDEN FUNCTIONS FOR LITERAL TYPES IN
    PYTHON SO THAT THE PROPERTIES BEHAVE WITHIN THE CODE
    AS IF THEY WERE THEIR LITERAL TYPES
    '''
    # Binary arithmetic operations
    __add__ = proxy_operator(_operator.add)
    __sub__ = proxy_operator(_operator.sub)
    __mul__ = proxy_operator(_operator.mul)
    __div__ = proxy_operator(_operator.div)
    __truediv__ = proxy_operator(_operator.truediv)
    __floordiv__ = proxy_operator(_operator.floordiv)
    __mod__= proxy_operator(_operator.mod)
    __divmod__ = proxy_operator(divmod)
    __pow__ = proxy_operator(pow)
    __lshift__ = proxy_operator(_operator.lshift)
    __rshift__ = proxy_operator(_operator.rshift)
    __and__ = proxy_operator(_operator.and_)
    __xor__ = proxy_operator(_operator.xor)
    __or__ = proxy_operator(_operator.or_)

    # In-place arithmetic operations
    __iadd__ = proxy_inplace_operator(_operator.add)
    __isub__ = proxy_inplace_operator(_operator.sub)
    __imul__ = proxy_inplace_operator(_operator.mul)
    __idiv__ = proxy_inplace_operator(_operator.div)
    __itruediv__ = proxy_inplace_operator(_operator.truediv)
    __ifloordiv__ = proxy_inplace_operator(_operator.floordiv)
    __imod__ = proxy_inplace_operator(_operator.mod)
    __ipow__ = proxy_inplace_operator(pow)
    __ilshift__ = proxy_inplace_operator(_operator.lshift)
    __irshift__ = proxy_inplace_operator(_operator.rshift)
    __iand__ = proxy_inplace_operator(_operator.and_)
    __ixor__ = proxy_inplace_operator(_operator.xor)
    __ior__ = proxy_inplace_operator(_operator.or_)

    # Reflected binary arithmetic operations
    __radd__ = proxy_reverse_operator(_operator.add)
    __rsub__ = proxy_reverse_operator(_operator.sub)
    __rmul__ = proxy_reverse_operator(_operator.mul)
    __rdiv__ = proxy_reverse_operator(_operator.div)
    __rtruediv__ = proxy_reverse_operator(_operator.truediv)
    __rfloordiv__ = proxy_reverse_operator(_operator.floordiv)
    __rmod__ = proxy_reverse_operator(_operator.mod)
    __rdivmod__ = proxy_reverse_operator(divmod)
    __rpow__ = proxy_reverse_operator(pow)
    __rlshift__ = proxy_reverse_operator(_operator.lshift)
    __rrshift__ = proxy_reverse_operator(_operator.rshift)
    __rand__ = proxy_reverse_operator(_operator.and_)
    __rxor__ = proxy_reverse_operator(_operator.xor)
    __ror__ = proxy_reverse_operator(_operator.or_)

    # Unary arithmetic operations
    __neg__ = proxy_operator(_operator.neg)
    __pos__ = proxy_operator(_operator.pos)
    __abs__ = proxy_operator(abs)
    __invert__ = proxy_operator(_operator.invert)

    # Type conversions
    __complex__ = proxy_operator(complex)
    __int__ = proxy_operator(int)
    __long__ = proxy_operator(long)
    __float__ = proxy_operator(float)
    __nonzero__ = proxy_operator(bool)

    # Base conversion
    __oct__ = proxy_operator(oct)
    __hex__ = proxy_operator(hex)

    # Coercion
    __coerce__ = proxy_operator(coerce)

    # Rich comparison methods
    __lt__ = proxy_operator(_operator.lt)
    __le__ = proxy_operator(_operator.le)
    __eq__ = proxy_operator(_operator.eq)
    __ne__ = proxy_operator(_operator.ne)
    __gt__ = proxy_operator(_operator.gt)
    __ge__ = proxy_operator(_operator.ge)

    # Sequence/string operations
    __len__ = proxy_operator(len)
    __getitem__ = proxy_operator(_operator.getitem)
    __contains__ = proxy_operator(_operator.contains)

    # Turn wrapper creation methods into static methods so that they can be
    # called by subclasses.
    proxy_operator = staticmethod(proxy_operator)
    proxy_inplace_operator = staticmethod(proxy_inplace_operator)
    proxy_reverse_operator = staticmethod(proxy_reverse_operator)
    proxy_modifier_function = staticmethod(proxy_modifier_function)
    
def _convertToComplex(value):
    if value == None:
        return None
    if isinstance(value.real, str):
        # for characters, we need to map to an integer as
        # the complex() method will not accept 2 strings as input
        value = complex(ord(value.real), ord(value.imag))
    else:
        value = complex(value.real, value.imag)
    return value
 
class simpleProperty(Property):
    def __init__(self, id, valueType, compRef, defValue=None, parent=None, structRef=None, structSeqRef=None, structSeqIdx=None, mode='readwrite', action='external'):
        """ 
        id - (string): the property ID
        valueType - (string): type of the property, must be in VALUE_TYPES
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        structRef - (string): name of the struct that this simple is a member of, or None
        structSeqRef - (string): name of the struct sequence the above struct is a member of, or None
        structSeqIdx - (int): index of the above struct in the struct sequence, or None
        mode - (string): mode for the property, must be in MODES
        """
        if valueType not in SCA_TYPES:
            raise('"' + str(valueType) + '"' + ' is not a valid valueType, choose from\n ' + str(SCA_TYPES))
        
        # Initialize the parent
        Property.__init__(self, id, type=valueType, compRef=compRef, mode=mode, action=action, parent=parent)
        
        self.valueType = valueType
        self.defValue = defValue
        
        self.typecode = getTypeCode(self.valueType)

        #used when the simple is part of a struct
        self.structRef = structRef
        
        # DEPRECATED: used when the struct is part of a struct sequence
        self.structSeqRef = structSeqRef
        self.structSeqIdx = structSeqIdx
        
        # DEPRECATED: create the CF.DataType reference, change value to the correct type
        self.propRef = _CF.DataType(id=str(self.id), value=_any.to_any(None))
        self.propRef.value._t = self.typecode

        
    def _queryValue(self):
        if self._parent:
            results = self._parent._queryValue()
            if results is None:
                return None
            for simple in results.value():
                if simple.id == self.id:
                    return simple.value
            return None
        else:
            # Standalone simple, do standard query.
            return super(simpleProperty,self)._queryValue()
            
    def _enumValue(self, value):
        for enumLabel, enumValue in _enums[self.id].iteritems():
            if value == enumLabel or value == enumValue:
                return enumValue
        raise ValueError, "Invalid enumeration value '%s'" % (value,)

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return None

        value = value.value()
        if self.valueType.find("complex") != -1:
            return _convertToComplex(value)
        else:
            return value

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value == None:
            return _any.to_any(None)

        # If property is an enumeration, enforce proper value
        if self.id in _enums.keys():
            value = self._enumValue(value)

        if self.valueType.startswith("complex"):
            memberTypeStr = mapComplexToSimple(self.valueType)
            real, imag = _type_helpers._splitComplex(value)
            realValue = _type_helpers.checkValidValue(real, memberTypeStr)
            imagValue = _type_helpers.checkValidValue(imag, memberTypeStr)

            # Convert to CORBA type (e.g., CF.complexFloat)
            value = getCFType(self.valueType)(realValue, imagValue)
        else:
            # Validate the value
            value = _type_helpers.checkValidValue(value, self.valueType)

        return _CORBA.Any(self.typecode, value)

    def _configureValue(self, value):
        if self._parent:
            # The simple is part of a struct.
            structValue = self._parent._queryValue()
            if structValue is None:
                return
            for simple in structValue._v:
                if simple.id == self.id:
                    simple.value = value
            self._parent._configureValue(structValue)
        else:
            # Standalone simple, do standard configure.
            super(simpleProperty,self)._configureValue(value)

    def __repr__(self, *args):
        value = self.queryValue()
        if value != None:
            print str(value),
        return ''
        
    def __str__(self, *args):
        value = self.queryValue()
        if value != None:
            return str(value)
        else:
            return ''
        
    def __getattr__(self, name):
        # If attribute is not found on simpleProperty, defer to the value; this
        # allows things like '.real' and '.imag', or string methods to work
        # without explicit support.
        return getattr(self.queryValue(), name)


class sequenceProperty(Property):
    def __init__(self, id, valueType, compRef, defValue=None, mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (string): type of the property, must be in VALUE_TYPES, or can be struct
        compRef - (domainless.componentBase) - pointer to the component that owns this property 
        mode - (string): mode for the property, must be in MODES
        
        This class inherits from list so that the property will behave
        as a list when the user wishes to manipulate it
        """
        if valueType not in SCA_TYPES and valueType != 'structSeq':
            raise('"' + str(valueType) + '"' + ' is not a valid valueType, choose from\n ' + str(SCA_TYPES))
        
        # Initialize the parent Property
        Property.__init__(self, id, type=valueType, compRef=compRef, mode=mode, action='external')
        
        self.defValue = defValue
        self.complex = False
        
        #try to set the value type in ref unless this is a sequence of structs
        if valueType != 'structSeq':
            self.valueType = valueType
            if self.valueType == "string":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.StringSeq)
            elif self.valueType == "boolean":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.BooleanSeq)
            elif self.valueType == "ulong":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ULongSeq)
            elif self.valueType == "short":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ShortSeq)
            elif self.valueType == "float":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.FloatSeq)
            elif self.valueType == "char":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.CharSeq)
            elif self.valueType == "octet":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.OctetSeq)
            elif self.valueType == "ushort":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.UShortSeq)
            elif self.valueType == "double":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.DoubleSeq)
            elif self.valueType == "long":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.LongSeq)
            elif self.valueType == "longlong":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.LongLongSequence)
            elif self.valueType == "ulonglong":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.UlongLongSequence) 
            elif self.valueType.find("complex") == 0:
                self.typecode = getCFSeqType(self.valueType)

                # It is important to have a means other than .find("complex")
                # to determine complexity, as in the case of a struct sequence,
                # the value of self.valueType may not be a string.
                self.complex = True
    
            # DEPRECATED: create the CF.DataType reference, change value to the correct type
            self.propRef = _CF.DataType(id=str(self.id), value=_any.to_any(None))
            self.propRef.value._t = self.typecode

    def _mapCFtoComplex(self, CFVal):
        return complex(CFVal.real, CFVal.imag)

    def _getComplexConfigValues(self, value):
        '''
        Go from:
            [complex(a,b), complex(a,b), ..., complex(a,b)]
        to:
            [CF.complexType(a,b), CF.complexType(a,b), ..., complexType(a,b)]
        '''

        memberTypeStr = mapComplexToSimple(self.valueType)
        newValues = []
        for val in value:
            # value is actually a list.  loop through each val
            # in the list and validate it
            realValue = _type_helpers.checkValidValue(val.real, 
                                                      memberTypeStr)
            imagValue = _type_helpers.checkValidValue(val.imag, 
                                                      memberTypeStr)
        
            # convert to CORBA type (e.g., CF.complexFloat)
            newValues.append(getCFType(self.valueType)(realValue, imagValue))
        return newValues

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return []

        values = value.value()
        if values is not None and values != '':
            if self.complex:
                values = [self._mapCFtoComplex(x) for x in values]
            elif self.valueType == 'octet':
                # Octet sequences are stored as strings
                values = [ord(x) for x in values]
            elif self.valueType == 'char':
                values = [x for x in values]
            return values
        if self.valueType == 'octet' or self.valueType == 'char':
            return ''
        return []

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value == None:
            return _any.to_any(None)

        if self.complex:
            value = self._getComplexConfigValues(value)
        else:
            value = _type_helpers.checkValidDataSet(value, self.valueType)

        return _CORBA.Any(self.typecode, value)
        
    '''
    BELOW ARE OVERRIDDEN FUNCTIONS FOR THE LIST CLASS IN
    PYTHON SO THAT THE PROPERTIES BEHAVE WITHIN THE CODE
    AS IF THEY ARE ACTUALLY LISTS
    '''
    # Container methods
    # __getitem__ and __contains__ are implemented in Property
    __setitem__ = Property.proxy_modifier_function(_operator.setitem)
    __delitem__ = Property.proxy_modifier_function(_operator.delitem)
    __iter__ = Property.proxy_operator(iter)
    # NB: __reversed__ is ignored prior to Python 2.6
    __reversed__ = Property.proxy_operator(reversed)

    # List methods
    append = Property.proxy_modifier_function(list.append)
    count = Property.proxy_operator(list.count)
    extend = Property.proxy_modifier_function(list.extend)
    index = Property.proxy_operator(list.index)
    insert = Property.proxy_modifier_function(list.insert)
    pop = Property.proxy_modifier_function(list.pop)
    remove = Property.proxy_modifier_function(list.remove)
    reverse = Property.proxy_modifier_function(list.reverse)
    sort = Property.proxy_modifier_function(list.sort)

    def __repr__(self):
        return repr(self.queryValue())
    
    def __str__(self):
        return str(self.queryValue())
    
            
class structProperty(Property):
    # All structs have the same CORBA typecode.
    typecode = _CORBA.TypeCode("IDL:CF/Properties:1.0")

    def __init__(self, id, valueType, compRef, defValue=None, parent=None, structSeqRef=None, structSeqIdx=None, mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (list): each entry in the list is a tuple defined in the following fashion:
                                (id, valueType(as defined for simples), defValue)
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        structSeqRef - (string) - name of the struct sequence that this struct is a part of, or None
        structSeqIdx - (int) - index of the struct  int the struct sequence, or None
        mode - (string): mode for the property, must be in MODES
        """
        if type(valueType) != list:
            raise('valueType must be provided as a list')
        self.valueType = valueType
        self.defValue = defValue
        
        #used when the struct is part of a struct sequence
        self.structSeqRef = structSeqRef
        self.structSeqIdx = structSeqIdx
        
        #initialize the parent
        Property.__init__(self, id, type='struct', compRef=compRef, mode=mode, action='external', parent=parent)
        
        #each of these members is itself a simple property
        self.members = {}
        for _id, _type, _defValue, _clean_name in valueType:
            if self.structSeqRef:
                simpleProp = simpleProperty(_id, _type, compRef=compRef, defValue=_defValue, parent=self, structRef=id, structSeqRef=self.structSeqRef, structSeqIdx=self.structSeqIdx)
                simpleProp.clean_name = _clean_name
            else:
                simpleProp = simpleProperty(_id, _type, compRef=compRef, defValue=_defValue, parent=self, structRef=id)
                simpleProp.clean_name = _clean_name
            self.members[_id] = (simpleProp)
        
        # DEPRECATED: create the CF.DataType reference        
        self.propRef = _CF.DataType(id=str(self.id), value=_CORBA.Any(self.typecode, None))

    def _queryValue(self):
        if self._parent:
            # Get the full struct sequence value.
            results = self._parent._queryValue()
            if results is None:
                return None
            return results.value()[self.structSeqIdx]
        else:
            # Standalone struct, do standard query.
            return super(structProperty,self)._queryValue()

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return {}

        structVal = {}
        for simple in value.value():
            member = self.members[simple.id]
            structVal[simple.id] = member.fromAny(simple.value)
        return structVal

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value is None:
            props = [_CF.DataType(str(m.id), m.toAny(None)) for m in self.members.values()]
            return _CORBA.Any(self.typecode, props)

        if not isinstance(value, dict):
            raise TypeError, 'configureValue() must be called with dict instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'

        # Check that the value passed in matches the struct definition
        self._checkValue(value)

        # Convert struct items into CF::Properties.
        props = []
        for _id, member in self.members.iteritems():
            memberVal = value.get(_id, member.defValue)
            props.append(_CF.DataType(str(_id), member.toAny(memberVal)))

        return _CORBA.Any(self.typecode, props)

    def _checkValue(self, value):
        for memberId in value:
            if memberId not in self.members:
                raise TypeError, "'%s' is not a member of '%s'" % (memberId, self.id)

    def _configureValue(self, value):
        # Check if this struct is a member of a struct sequence
        if self._parent:
            # Get the full struct sequence value.
            results = self._parent._queryValue()
            if results is None:
                return

            # Replace the struct in the list.
            results._v[self.structSeqIdx] = value

            # Configure the complete, updated struct sequence.
            self._parent._configureValue(results)
        else:
            # Standalone struct, do standard configure.
            super(structProperty,self)._configureValue(value)

    def configureValue(self, value):
        '''
        Helper function for configuring a struct property, using a
        dictionary as the passed in value
        '''
        if value is not None:
            # Fill in any missing member values from the current value.
            self._checkValue(value)
            for propId, propVal in self.queryValue().items():
                if propId not in value:
                    value[propId] = propVal
        super(structProperty,self).configureValue(value)
    
    def __str__(self):
        currValue = self.queryValue()
        structView = "ID: " + self.id
        for key in currValue:
            structView = structView + '\n  ' + str(self.members[key].clean_name) + ": " + str(currValue[key])
        return structView
    
    def __repr__(self):
        currValue = self.queryValue()
        structView = "ID: " + self.id
        for key in currValue:
            structView = structView + '\n  ' + str(self.members[key].clean_name) + ": " + str(currValue[key])
        print structView,
        return ''
    
    def __getattr__(self, name):
        '''
        If the attribute being looked up is actually a member of the struct,
        then return that simple property, otherwise default to the normal
        getattribute function
        '''
        try:
           return object.__getattribute__(self, "members")[_displayNames[self.compRef._refid][name]]
        except:
           return object.__getattribute__(self,name)
    
    def __setattr__(self, name, value):
        '''
        If the attribute being looked up is actually a member of the struct,
        then try to configure the simple property.  This will result in a
        configure of the entire struct in the simpleProperty class
        '''
        try:
            self.members[_displayNames[self.compRef._refid][name]].configureValue(value)
        except AttributeError:
            return object.__setattr__(self, name, value)
        except KeyError:
            return object.__setattr__(self, name, value)
            
        
class structSequenceProperty(sequenceProperty):
    # All struct sequences have the same CORBA typecode.
    typecode = _CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0")

    def __init__(self, id, structID, valueType, compRef, defValue=[], mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (list): each entry in the list is a tuple defined in the following fashion:
                                (id, valueType(as defined for simples), defValue)
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        mode - (string): mode for the property, must be in MODES
        """
        if type(valueType) != list:
            raise('valueType must be provided as a list')
        self.valueType = valueType
        self.structID = structID
        self.defValue = defValue
        
        #initialize the parent
        sequenceProperty.__init__(self, id, valueType='structSeq', compRef=compRef, defValue=self.defValue, mode=mode)

        # Create a property for the struct definition.
        self.structDef = structProperty(id=self.structID, valueType=self.valueType, compRef=self.compRef, mode=self.mode)

        # DEPRECATED: Create the CF.DataType reference
        self.propRef = _CF.DataType(id=str(self.id), value=_CORBA.Any(self.typecode, []))

    def __getitem__(self, index):
        #the actual struct property doesn't exist, so create it and return it
        newProp = structProperty(id=self.structID, valueType=self.valueType, compRef=self.compRef, \
                                 parent=self, structSeqRef=self.id, structSeqIdx=index, mode=self.mode)
        return newProp
    
    def __setitem__(self, index, value):
        #the actual struct property doesn't exist, so create it and configure it,
        #this will trigure a configure of the entire sequence from within structProperty
        newProp = structProperty(id=self.structID, valueType=self.valueType, compRef=self.compRef, \
                                 parent=self, structSeqRef=self.id, structSeqIdx=index, mode=self.mode)
        structProperty.configureValue(newProp, value)

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return []

        return [self.structDef.fromAny(v) for v in value.value()]

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value is None:
            return _any.to_any(None)

        return _CORBA.Any(self.typecode, [self.structDef.toAny(v) for v in value])

def parseComplexString(ajbString, baseType):
    '''
    Parse a string in the form A+jB into its real (A) and imag (B) components.

    baseType can either be a Python type (e.g., float) , or a string
    containing the name of a complex type (e.g., "float").
    '''

    if __TYPE_MAP.has_key(baseType): 
        # if the type is passed in as a string
        # e.g., "float" vs. float
        baseType = getPyType(baseType) 

    real = baseType(0)
    imag = baseType(0)
    sign = 1
    signLocation = ajbString.find("-j")
    if signLocation != -1:
        # if a negative-sign is found for imag data
        sign = -1
    jLocation = ajbString.find("j")
    if jLocation < 0:
        # if no "j", then we just have a real number
        real = baseType(ajbString)
    else:
        real = baseType(ajbString[:jLocation-1])
        imag = sign * baseType(ajbString[jLocation+1:])
    return real, imag
