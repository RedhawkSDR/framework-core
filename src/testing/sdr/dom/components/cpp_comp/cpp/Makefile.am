CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = cpp_comp

cpp_comp_SOURCES = cpp_comp.cpp cpp_comp.h cpp_comp_base.cpp cpp_comp_base.h main.cpp
cpp_comp_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
cpp_comp_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la
