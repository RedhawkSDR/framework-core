#!/usr/bin/env python
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

#
from ossie.cf import CF, CF__POA
from ossie.resource import Resource, start_component, usesport
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property
from ossie.events import PropertyEventSupplier
import time
import logging
import copy

class PropertyChangeEvents(CF__POA.Resource, Resource):
    """
    Example component to send property change events to an EventChannel port.
    """

    myprop = simple_property(id_="myprop",
                             type_="long",
                             name="myprop",
                             defvalue=0,
                             mode="readwrite",
                             action="external",
                             kinds=("configure","event"))

    anotherprop = simple_property(id_="anotherprop",
                             type_="long",
                             name="anotherprop",
                             defvalue=0,
                             mode="readwrite",
                             action="external",
                             kinds=("configure",))

    propEvent = usesport(name="propEvent",
                         repid="IDL:CosEventChannelAdmin/EventChannel:1.0",
                         type_="data")

    seqprop = simpleseq_property(id_="seqprop",  
                                        type_="float",
                                        defvalue=None,
                                        mode="readwrite",
                                        action="external",
                                        kinds=("configure","event")
                                        )
    class SomeStruct(object):
        some_number = simple_property(id_="some_number",
                                        type_="double",
                                        )
        some_string = simple_property(id_="some_string",
                                        type_="string",
                                        )

        def __init__(self):
            """Construct an initialized instance of this struct definition"""
            for attrname, classattr in type(self).__dict__.items():
                if type(classattr) == simple_property:
                    classattr.initialize(self)

        def __str__(self):
            """Return a string representation of this structure"""
            d = {}
            d["some_number"] = self.some_number
            d["some_string"] = self.some_string
            return str(d)

        def isStruct(self):
            return True

        def getMembers(self):
            return [("some_number",self.some_number),("some_string",self.some_string)]

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)

    def initialize(self):
        self.propEvent = PropertyEventSupplier(self)


    some_struct = struct_property(id_="some_struct",
                                        structdef=SomeStruct,
                                        configurationkind=("configure","event"),
                                        mode="readwrite"
                                        )
    structseq_prop = structseq_property(id_="structseq_prop",
                                        structdef=SomeStruct,
                                        defvalue=[],
                                        configurationkind=("configure","event"),
                                        mode="readwrite"
                                        )
    
    def start(self):
        self.myprop = 123
        self.anotherprop = 123
        self.seqprop = [1.0]
        self.some_struct.some_number = 123.0
        tmp = self.SomeStruct()
        tmp.some_number = 2.0
        tmp.some_string = "another string"
        newval = copy.deepcopy(self.structseq_prop)
        newval.append(tmp)
        self.structseq_prop = newval


if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    start_component(PropertyChangeEvents)
