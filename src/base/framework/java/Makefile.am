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

# vim: noet: softtabstop=0

# Because omniidl is definitely not capable
.NOTPARALLEL:

SUBDIRS = jni . ossie

CF_IDLDIR = $(top_srcdir)/idl

# The COS modules are mapped to the org.omg package.
COSPKGS = CosEventComm CosNaming CosEventChannelAdmin
COSPREFIX = org.omg

# Suppress warnings due to unrecognized omniORB pragmas in COS packages
IDLJFLAGS = -noWarn $(COSPKGS:%=-pkgPrefix % $(COSPREFIX)) -i $(IDLDIR) -i $(OMNICOS_IDLDIR)

IDLJNIFLAGS = -Wblibname=ossiecfjni $(COSPKGS:%=-Wbpkgprefix=%:$(COSPREFIX))

cf/src:
	mkdir -p cf/src

# Generate Java classes 
%.idlj : $(CF_IDLDIR)/ossie/CF/%.idl | cf/src
	$(IDLJ) -fall -td cf/src $(IDLJFLAGS) -i $(CF_IDLDIR) $<
	$(IDLJ) -fallTie -td cf/src $(IDLJFLAGS) -i $(CF_IDLDIR) $<
	touch $@

# Generate Java JNI classes
%.omnijni : $(CF_IDLDIR)/ossie/CF/%.idl | cf/src
	$(OMNIJNI_IDLJAVA) -C cf/src $(IDLJNIFLAGS) -I $(CF_IDLDIR) -I $(IDLDIR) -I $(OMNICOS_IDLDIR) $<
	touch $@

# The ExtendedEvent IDL depends on COS packages that are not provided in the
# default JRE CORBA implementation, so they must be compiled into our jar.
Cos%.idlj : $(OMNICOS_IDLDIR)/Cos%.idl | cf/src
	$(IDLJ) -fall -td cf/src $(IDLJFLAGS) $<
	$(IDLJ) -fallTIE -td cf/src $(IDLJFLAGS) $<
	touch $@

Cos%.omnijni: $(OMNICOS_IDLDIR)/Cos%.idl | cf/src
	$(OMNIJNI_IDLJAVA) -C cf/src $(IDLJNIFLAGS) -I$(OMNICOS_IDLDIR) $<
	touch $@

IDLSRC = CosEventComm.idl CosEventChannelAdmin.idl \
	 LogInterfaces.idl \
	 cf.idl PortTypes.idl StandardEvent.idl AggregateDevices.idl \
	 ExtendedEvent.idl QueryablePort.idl WellKnownProperties.idl sandbox.idl

# CosNaming is included with the JRE, so only build JNI bindings.
JNISRC = CosNaming.idl $(IDLSRC)

IDLJFILES = $(IDLSRC:.idl=.idlj)
JNIFILES = $(JNISRC:.idl=.omnijni)

cf/src/fileList.txt : $(JNIFILES) $(IDLJFILES)
	find cf/src -name "*.java" > $@

CFInterfaces.jar: cf/src/fileList.txt
	rm -rf cf/bin
	mkdir -p cf/bin
	$(JAVAC) -source 1.5 -g -cp apache-commons-lang-2.4.jar:$(omnijni_builddir)/src/java -d cf/bin @$<
	$(JAR) cf $@ -C cf/bin .
	$(JAR) uf $@ -C cf/src .
	$(JAR) uf $@ -C $(omnijni_builddir)/src/java omnijni

java_DATA = CFInterfaces.jar log4j-1.2.15.jar apache-commons-lang-2.4.jar
CLEANFILES = $(IDLJFILES) $(JNIFILES) CFInterfaces.jar cf/src/fileList.txt

clean-local:
	rm -rf cf
