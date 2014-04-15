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
	$(AM_V_GEN)$(IDLJ) -fall -td cf/src $(IDLJFLAGS) -i $(CF_IDLDIR) $<
	$(AM_V_GEN)$(IDLJ) -fallTie -td cf/src $(IDLJFLAGS) -i $(CF_IDLDIR) $<
	$(AM_V_at)touch $@

# Generate Java JNI classes
%.omnijni : $(CF_IDLDIR)/ossie/CF/%.idl | cf/src
	$(AM_V_GEN)$(OMNIJNI_IDLJAVA) -C cf/src $(IDLJNIFLAGS) -I $(CF_IDLDIR) -I $(IDLDIR) -I $(OMNICOS_IDLDIR) $<
	$(AM_V_at)touch $@

# The ExtendedEvent IDL depends on COS packages that are not provided in the
# default JRE CORBA implementation, so they must be compiled into our jar.
Cos%.idlj : $(OMNICOS_IDLDIR)/Cos%.idl | cf/src
	$(AM_V_GEN)$(IDLJ) -fall -td cf/src $(IDLJFLAGS) $<
	$(AM_V_GEN)$(IDLJ) -fallTIE -td cf/src $(IDLJFLAGS) $<
	$(AM_V_at)touch $@

Cos%.omnijni: $(OMNICOS_IDLDIR)/Cos%.idl | cf/src
	$(AM_V_GEN)$(OMNIJNI_IDLJAVA) -C cf/src $(IDLJNIFLAGS) -I$(OMNICOS_IDLDIR) $<
	$(AM_V_at)touch $@

IDLSRC = CosEventComm.idl CosEventChannelAdmin.idl \
	 LogInterfaces.idl \
	 cf.idl PortTypes.idl StandardEvent.idl AggregateDevices.idl \
	 ExtendedEvent.idl QueryablePort.idl WellKnownProperties.idl sandbox.idl

# CosNaming is included with the JRE, so only build JNI bindings.
JNISRC = CosNaming.idl $(IDLSRC)

IDLJFILES = $(IDLSRC:.idl=.idlj)
JNIFILES = $(JNISRC:.idl=.omnijni)

.INTERMEDIATE: META-INF/MANIFEST.MF fileList.txt packageList.txt

fileList.txt : $(JNIFILES) $(IDLJFILES)
	$(AM_V_at)find cf/src -name "*.java" > $@

packageList.txt: $(JNIFILES) $(IDLJFILES)
	$(AM_V_at)echo -n "Export-Package:" > $@
	$(AM_V_at)find cf/src -mindepth 1 -type d | $(SED) -e 's/^cf\/src\// /' -e 's/\//\./g' -e '$$ ! s/$$/,/' >> $@

META-INF/MANIFEST.MF: META-INF/MANIFEST.MF.in packageList.txt Makefile
	$(AM_V_GEN)stamp=`date +%Y%m%d%H%M`; \
	$(SED) \
	  -e '/@EXPORT_LIST[@]/r packageList.txt' \
	  -e '/@EXPORT_LIST[@]/d' \
	  -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|' \
	  -e "s|@DATE_TIME_STAMP[@]|$$stamp|" \
	  $@.in > $@

CFInterfaces.jar: fileList.txt META-INF/MANIFEST.MF
	$(AM_V_at)rm -rf cf/bin
	$(AM_V_at)mkdir -p cf/bin
	$(JAVAC) -source 1.5 -g -cp apache-commons-lang-2.4.jar:$(omnijni_builddir)/src/java -d cf/bin @$<
	$(JAR) cMf $@ META-INF/MANIFEST.MF -C cf/bin .
	$(JAR) uf $@ -C cf/src .
	$(JAR) uf $@ -C $(omnijni_builddir)/src/java omnijni

java_DATA = CFInterfaces.jar log4j-1.2.15.jar apache-commons-lang-2.4.jar
CLEANFILES = $(IDLJFILES) $(JNIFILES) CFInterfaces.jar

clean-local:
	rm -rf cf
