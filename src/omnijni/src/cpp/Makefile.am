SUBDIRS = CORBA

lib_LTLIBRARIES = libomnijni.la

libomnijni_la_CPPFLAGS = $(OMNIORB_CFLAGS) $(OMNIJNI_CPPFLAGS)
libomnijni_la_SOURCES = any.cpp \
			exception.cpp \
			object.cpp \
			omnijni.cpp \
			orb.cpp \
			sequence.cpp \
			utilities.cpp
libomnijni_la_LIBADD = $(OMNIORB_LIBS) CORBA/libcorbajni.la
