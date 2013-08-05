import logging
import threading

import CosNaming, CosNaming__POA

from omniORB import CORBA, URI
from ossie.utils import log4py

log = logging.getLogger(__name__)

class NamingContextStub(CosNaming__POA.NamingContextExt):
    """
    Class used to generate a NamingContext object.  It extends the 
    NamingContextExt rather than the NameContext so it can work with Java as 
    well with the other implementations.  
    
    The only additional method that had to be implemented to support this 
    parent class was the to_name(name) method.  Java implementations invoke 
    this method to generate a list of NamingComponent objects based on some
    string format explained below.
     
    """
    def __init__(self):
        self.__context = {}
        self.__lock = threading.RLock()

    def bind(self, name, object):
        uri = URI.nameToString(name)
        self.__lock.acquire()
        try:
            if uri in self.__context:
                raise CosNaming.NamingContext.AlreadyBound()
            log.debug('Binding "%s" into virtual NamingContext', uri)
            self.__context[uri] = object
        finally:
            self.__lock.release()

    def rebind(self, name, object):
        uri = URI.nameToString(name)
        log.debug('Rebinding "%s" into virtual NamingContext', uri)
        self.__lock.acquire()
        try:
            self.__context[uri] = object
        finally:
            self.__lock.release()

    def getObject(self, name):
        self.__lock.acquire()
        try:
            return self.__context.get(name, None)
        finally:
            self.__lock.release()

    def to_name(self, name):
        """
        Returns a list of CosNaming.NameComponent from the incoming string.
        
        The name is a string used to generate NameComponent objects.  The 
        NameComponent object takes 2 optional arguments: id and kind.  The 
        string can have one or more names as follow:
        
            id1.kind1/id2.kind2/id3./.kind4
        
        Where the first two objects have the id and the kind.  The third object
        does not have a kind defined, and the fourth object does not have an 
        id.  Both, the id and the kind, can use a '\' as a escape character to
        include either the '.' or the '/' as part of them.  For instance the 
        following pair is valid:
        
            my\.id.my\/kind ==> ("my.id" "my/kind")
        
        If an error is found while parsing the string, then a ValueError is 
        raised and an empty list is returned.
        
        """
        names = []
        # Before splitting the name, replace all the escapes
        data = {'DOT_TOKEN':'\.', 'SLASH_TOKEN':'\/'}

        try:
            # if a escape dot is found replace it with the DOT_TOKEN
            name = name.replace('\.', '%(DOT_TOKEN)s')
            # if a escape slash is found replace it with the SLASH_TOKEN
            name = name.replace('\/', '%(SLASH_TOKEN)s')

            # now that we are not 'escaping' we can split the name into what 
            # will become NameComponent objects
            items = name.split('/')
            # for evert NameComponent, get the id and the kind
            for item in items:
                # Separate the id and the kind
                end = item.find('.')
                if end >= 0:
                    id_ = item[0:end]
                    kind_ = item[end + 1:]
                    # replace the DOT_TOKEN and the SLASH_TOKEN back 
                    id_ = id_ % data
                    kind_ = kind_ % data
                else:
                    id_ = item
                    kind_ = ''
                # create the NameComponent object and append it to the list
                names.append(CosNaming.NameComponent(id_, kind_))

            return names
        except:
            raise ValueError("NamingContextStub:to_name() '%s' is an invalid name" % name)

        return []
