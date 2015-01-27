if HAVE_JAVASUPPORT

java_dev.jar:
	mkdir -p bin
	find ./src -name "*.java" > fileList.txt
	$(JAVAC) -cp $(OSSIE_CLASSPATH) -d bin @fileList.txt
	$(JAR) cf ./java_dev.jar -C bin .
	rm fileList.txt

clean-local:
	rm -rf bin 

java_dev_jar_SOURCES := $(shell find ./src -name "*.java")

ossieName = java_dev
noinst_PROGRAMS = java_dev.jar

else

all-local:
	@echo "Java support disabled - java_dev will not be compiled"

endif
