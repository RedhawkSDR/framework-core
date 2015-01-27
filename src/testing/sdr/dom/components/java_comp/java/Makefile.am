if HAVE_JAVASUPPORT

java_comp.jar:
	mkdir -p bin
	find ./src -name "*.java" > fileList.txt
	$(JAVAC) -cp $(OSSIE_CLASSPATH) -d bin @fileList.txt
	$(JAR) cf ./java_comp.jar -C bin .
	rm fileList.txt

clean-local:
	rm -rf bin 

java_comp_jar_SOURCES := $(shell find ./src -name "*.java")

ossieName = java_comp
noinst_PROGRAMS = java_comp.jar

else

all-local:
	@echo "Java support disabled - java_comp will not be compiled"

endif
