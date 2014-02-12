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

# NB: In order to create a large number of strongly-typed property classes that
#     differ only in the numeric type, common templates and a small Python
#     script are used to generate each instance. This allows us to make changes
#     in a single location if the base classes change, and reduces the risk of
#     a typo in a single class; however it somewhat complicates the build of
#     the jarfile.

# Numeric property classes
SIMPLE_PROPERTIES = src/org/ossie/properties/DoubleProperty.java \
		    src/org/ossie/properties/FloatProperty.java \
		    src/org/ossie/properties/LongLongProperty.java \
		    src/org/ossie/properties/LongProperty.java \
		    src/org/ossie/properties/OctetProperty.java \
		    src/org/ossie/properties/ShortProperty.java \
		    src/org/ossie/properties/ULongLongProperty.java \
		    src/org/ossie/properties/ULongProperty.java \
		    src/org/ossie/properties/UShortProperty.java

$(SIMPLE_PROPERTIES) : src/org/ossie/properties/NumericProperty.template
	(cd src/org/ossie/properties && $(PYTHON) generate-props.py)

# Numeric sequence property classes
SEQUENCE_PROPERTIES = src/org/ossie/properties/DoubleSequenceProperty.java \
		      src/org/ossie/properties/FloatSequenceProperty.java \
		      src/org/ossie/properties/LongLongSequenceProperty.java \
		      src/org/ossie/properties/LongSequenceProperty.java \
		      src/org/ossie/properties/OctetSequenceProperty.java \
		      src/org/ossie/properties/ShortSequenceProperty.java \
		      src/org/ossie/properties/ULongLongSequenceProperty.java \
		      src/org/ossie/properties/ULongSequenceProperty.java \
		      src/org/ossie/properties/UShortSequenceProperty.java

$(SEQUENCE_PROPERTIES) : src/org/ossie/properties/NumericSequenceProperty.template
	(cd src/org/ossie/properties && $(PYTHON) generate-props.py)

# Complex simple property classes
COMPLEX_PROPERTIES = src/org/ossie/properties/ComplexBooleanProperty.java \
		     src/org/ossie/properties/ComplexCharProperty.java \
		     src/org/ossie/properties/ComplexDoubleProperty.java \
		     src/org/ossie/properties/ComplexFloatProperty.java \
		     src/org/ossie/properties/ComplexLongLongProperty.java \
		     src/org/ossie/properties/ComplexLongProperty.java \
		     src/org/ossie/properties/ComplexOctetProperty.java \
		     src/org/ossie/properties/ComplexShortProperty.java \
		     src/org/ossie/properties/ComplexULongLongProperty.java \
		     src/org/ossie/properties/ComplexULongProperty.java \
		     src/org/ossie/properties/ComplexUShortProperty.java

$(COMPLEX_PROPERTIES) : src/org/ossie/properties/ComplexProperty.template
	(cd src/org/ossie/properties && $(PYTHON) generate-props.py)

# Complex sequence property classes
COMPLEX_SEQUENCES = src/org/ossie/properties/ComplexBooleanSequenceProperty.java \
		    src/org/ossie/properties/ComplexCharSequenceProperty.java \
		    src/org/ossie/properties/ComplexDoubleSequenceProperty.java \
		    src/org/ossie/properties/ComplexFloatSequenceProperty.java \
		    src/org/ossie/properties/ComplexLongLongSequenceProperty.java \
		    src/org/ossie/properties/ComplexLongSequenceProperty.java \
		    src/org/ossie/properties/ComplexOctetSequenceProperty.java \
		    src/org/ossie/properties/ComplexShortSequenceProperty.java \
		    src/org/ossie/properties/ComplexULongLongSequenceProperty.java \
		    src/org/ossie/properties/ComplexULongSequenceProperty.java \
		    src/org/ossie/properties/ComplexUShortSequenceProperty.java

$(COMPLEX_SEQUENCES) : src/org/ossie/properties/ComplexSequenceProperty.template
	(cd src/org/ossie/properties && $(PYTHON) generate-props.py)

# Regular source files
OSSIE_SOURCE = src/org/ossie/component/AllocCapacity.java \
	       src/org/ossie/component/Device.java \
	       src/org/ossie/component/ProvidesPort.java \
	       src/org/ossie/component/QueryableUsesPort.java \
	       src/org/ossie/component/Resource.java \
	       src/org/ossie/component/Service.java \
	       src/org/ossie/component/UsesPort.java \
	       src/org/ossie/logging/logging.java \
	       src/org/ossie/events/Consumer_i.java \
	       src/org/ossie/events/EventCallback.java \
	       src/org/ossie/events/MessageConsumerPort.java \
	       src/org/ossie/events/MessageSupplierPort.java \
	       src/org/ossie/events/PropertyEventSupplier.java \
	       src/org/ossie/events/SupplierAdmin_i.java \
	       src/org/ossie/properties/AbstractSequenceProperty.java \
	       src/org/ossie/properties/AbstractSimpleProperty.java \
	       src/org/ossie/properties/Action.java \
	       src/org/ossie/properties/Allocator.java \
	       src/org/ossie/properties/AnyUtils.java \
	       src/org/ossie/properties/BooleanProperty.java \
	       src/org/ossie/properties/BooleanSequenceProperty.java \
	       src/org/ossie/properties/CharProperty.java \
	       src/org/ossie/properties/CharSequenceProperty.java \
	       src/org/ossie/properties/ComplexArrayUtils.java \
	       src/org/ossie/properties/ComplexUtils.java \
	       src/org/ossie/properties/IProperty.java \
	       src/org/ossie/properties/Kind.java \
	       src/org/ossie/properties/LegacyProperty.java \
	       src/org/ossie/properties/Mode.java \
	       src/org/ossie/properties/NumericProperty.java \
	       src/org/ossie/properties/NumericSequenceProperty.java \
	       src/org/ossie/properties/PrimitiveArrayUtils.java \
	       src/org/ossie/properties/Property.java \
	       src/org/ossie/properties/PropertyListener.java \
	       src/org/ossie/properties/StringProperty.java \
	       src/org/ossie/properties/StringSequenceProperty.java \
	       src/org/ossie/properties/StructDef.java \
	       src/org/ossie/properties/StructProperty.java \
	       src/org/ossie/properties/StructSequenceProperty.java \
	       src/org/ossie/properties/UnsignedUtils.java

# Deprecated generic property classes, replaced by strongly-typed classes but
# maintained for source compatibility.
OSSIE_SOURCE += src/org/ossie/properties/SimpleProperty.java \
		src/org/ossie/properties/SimpleSequenceProperty.java

OSSIE_SOURCE += $(SIMPLE_PROPERTIES)
OSSIE_SOURCE += $(SEQUENCE_PROPERTIES)
OSSIE_SOURCE += $(COMPLEX_PROPERTIES)
OSSIE_SOURCE += $(COMPLEX_SEQUENCES)

src/fileList.txt : $(OSSIE_SOURCE)
	echo $^ > src/fileList.txt

# Additional Java compiler flags
CLASSPATH = ../apache-commons-lang-2.4.jar:../CFInterfaces.jar:../log4j-1.2.15.jar
JAVAFLAGS = -source 1.5 -g -Xlint:deprecation -Xlint:unchecked -cp $(CLASSPATH)

ossie.jar : src/fileList.txt
	rm -rf bin
	mkdir -p bin
	$(JAVAC) $(JAVAFLAGS) -d bin @src/fileList.txt
	$(JAR) cf ossie.jar -C bin  .
	$(JAR) uf ossie.jar -C src  .

java_DATA = ossie.jar

CLEANFILES = ossie.jar src/fileList.txt
CLEANFILES += $(SIMPLE_PROPERTIES)
CLEANFILES += $(SEQUENCE_PROPERTIES)
CLEANFILES += $(COMPLEX_PROPERTIES)
CLEANFILES += $(COMPLEX_SEQUENCES)

clean-local:
	rm -rf bin
