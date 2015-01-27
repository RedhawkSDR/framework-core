CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = cpp_dev

cpp_dev_SOURCES = cpp_dev.cpp cpp_dev.h cpp_dev_base.cpp cpp_dev_base.h main.cpp
cpp_dev_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
cpp_dev_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la

