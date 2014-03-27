CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = CppCallbacks

CppCallbacks_SOURCES = CppCallbacks.cpp CppCallbacks.h CppCallbacks_base.cpp CppCallbacks_base.h main.cpp struct_props.h
CppCallbacks_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
CppCallbacks_LDADD = $(BOOST_REGEX_LIB) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la

