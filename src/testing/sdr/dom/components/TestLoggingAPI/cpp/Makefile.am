CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = TestLoggingAPI

TestLoggingAPI_SOURCES = TestLoggingAPI.cpp TestLoggingAPI.h TestLoggingAPI_base.cpp TestLoggingAPI_base.h main.cpp struct_props.h
TestLoggingAPI_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
TestLoggingAPI_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la

