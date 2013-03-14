# REDHAWK Core Framework Ubuntu Instructions

## Install Dependencies

    sudo apt-get install build-essential \
                         openjdk-6-jdk \
                         python-omniorb \
                         libboost1.46-dev \
                         omnievents \
                         omniidl \
                         omniidl-python \
                         omniorb \
                         omniorb-idl \
                         omniorb-nameserver \
                         libomnievents-dev \
                         libomniorb4-dev \
                         xsdcxx \
                         python-numpy \
                         liblog4cxx10-dev
                         
## Setup JAVA_HOME

The REDHAWK runtime and build process expect the JAVA_HOME environment variable
to be set.  Set this to a JDK installed in /usr/lib/jvm. For example:

    export JAVA_HOME /usr/lib/jvm/java-6-openjdk-amd64

You may prefer to set this in your ~/.bashrc or equivalent.

## Running the build

Use the build_src.sh script or within the src folder execute:
    ./reconf
    ./configure
    make

## Setting up /etc/omniORB.cfg

The default Ubuntu configuration needs modifications to work with the baseline
REDHAWK framework.

1. Increase the giopMaxMessageSize to 10485760 (i.e. 10MB)
2. Comment out the line "DefaultInitRef = corbaloc::"
3. Add the line "InitRef = NameService=corbaname::localhost"
4. Add the line "EventService=corbaloc::localhost:11169/omniEvents"

## Known Issues

1. The omnievent server has a bug in it that causes it to crash while running the
associated unittests (i.e. test_08_*).
