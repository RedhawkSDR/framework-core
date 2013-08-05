#!/bin/sh
myDir=`dirname $0`

# Setup the OSSIEHOME Lib jars on the classpath
libDir=../base/framework/java
libFiles=`ls -1 $libDir/*.jar`
for file in $libFiles
do
	if [ x"$CLASSPATH" = "x" ]
	then
		export CLASSPATH=$file
	else
		export CLASSPATH=$file:$CLASSPATH
	fi
done

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly
myDir=`dirname $0`

#Sun ORB start line
exec $JAVA_HOME/bin/java -cp ::$myDir/TestComplexProps.jar:$myDir/bin:$CLASSPATH TestComplexProps.java.TestComplexProps "$@"

