CFDIR = $(top_srcdir)/base
INCLUDES = -I$(CFDIR)/include

noinst_PROGRAMS = CppCallbacks

CppCallbacks_SOURCES = CppCallbacks.cpp CppCallbacks.h CppCallbacks_base.cpp CppCallbacks_base.h main.cpp struct_props.h
CppCallbacks_CXXFLAGS = -Wall $(BOOST_CPPFLAGS)
CppCallbacks_LDADD = $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la
