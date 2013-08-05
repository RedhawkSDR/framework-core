JavaTestDevice.jar: $(JavaTestDevice_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(OSSIE_CLASSPATH) -d bin $(JavaTestDevice_jar_SOURCES)
	$(JAR) cf ./JavaTestDevice.jar -C bin .

clean-local:
	rm -rf bin

distclean-local:
	rm -f config.*
	rm -rf autom4te.cache
	rm -f acinclude.m4
	rm -f aclocal.m4
	rm -f configure
	rm -f depcomp
	rm -f install-sh
	rm -f ltmain.sh
	rm -f Makefile.in
	rm -f missing
	rm -rf .deps

ossieName = JavaTestDevice
bindir = $(prefix)/dev/devices/JavaTestDevice/java/
noinst_PROGRAMS = JavaTestDevice.jar
JavaTestDevice_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dev/devices/JavaTestDevice/

devdir = $(prefix)/dev/devices/JavaTestDevice/java/
