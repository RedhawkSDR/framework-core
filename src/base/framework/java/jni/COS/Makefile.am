# Build a separate library for JNI interface to COS; it will be linked directly
# into ossiecfjni rather than installed. The headers, however, are installed to
# ${includedir}/ossie/COS.
noinst_LTLIBRARIES = libcosjni.la

jni_CosEvent%.cpp jni_CosEvent%.h : $(OMNICOS_IDLDIR)/CosEvent%.idl
	$(OMNIJNI_IDLJNI) -Wbincprefix=COS -Wbincext=.hh -Wbpkgprefix=CosEventComm:COS -I$(OMNICOS_IDLDIR) $<

jni_CosNam%.cpp jni_CosNam%.h : $(OMNICOS_IDLDIR)/CosNam%.idl
	$(OMNIJNI_IDLJNI) -Wbstandalone -Wbpkgprefix=CosNaming:COS -I$(OMNICOS_IDLDIR) $<

jni_CosEventChannelAdmin.o : jni_CosEventComm.h

nodist_libcosjni_la_SOURCES = jni_CosEventComm.cpp \
			      jni_CosEventChannelAdmin.cpp \
			      jni_CosNaming.cpp
libcosjni_la_CPPFLAGS = $(OMNIJNI_CPPFLAGS)

cosincludedir = $(pkgincludedir)/COS
nodist_cosinclude_HEADERS = jni_CosEventComm.h \
			    jni_CosEventChannelAdmin.h \
			    jni_CosNaming.h

CLEANFILES = jni_*.h jni_*.cpp
