# REDHAWK Core Framework Ubuntu Instructions

## Install Dependencies

    sudo apt-get install build-essential \
                         openjdk-6-jdk \
                         python-omniorb \
                         libboost-dev \
                         libboost-system-dev \
                         libboost-filesystem-dev \
                         libboost-regex-dev \
                         libboost-thread-dev \
                         omnievents \
                         omniidl \
                         omniidl-python \
                         omniorb \
                         omniorb-idl \
                         omniorb-nameserver \
                         libcos4-dev \
                         libomnievents-dev \
                         libomniorb4-dev \
                         xsdcxx \
                         python-numpy \
                         python-omniorb \
                         omniidl-python \
                         liblog4cxx10-dev
                         
## Setup JAVA_HOME

The REDHAWK runtime and build process expect the JAVA_HOME environment variable
to be set.  Set this to a JDK installed in /usr/lib/jvm. For example:

    export JAVA_HOME /usr/lib/jvm/java-6-openjdk-amd64

You may prefer to set this in your ~/.bashrc or equivalent.

## Running the build

Use the build_src.sh script or within the src folder execute:

    ./reconf
    ./configure --with-ossie=/usr/local/redhawk/core --with-sdr=/var/redhawk/sdr
    make

## Setting up /etc/omniORB.cfg

The default Ubuntu configuration needs modifications to work with the baseline
REDHAWK framework.

1. Increase the giopMaxMsgSize to 10485760 (i.e. 10MB)
2. Comment out the line "DefaultInitRef = corbaloc::"
3. Add the line "InitRef = NameService=corbaname::localhost"
4. Add the line "EventService=corbaloc::localhost:11169/omniEvents"

## Installation

Installation can be performed by running:

    sudo make install

However, you may prefer to use the checkinstall tool so that you can easily uninstall.

    sudo checkinstall --provides=redhawk --pkgversion=1.8.3 --pkgname=redhawk make install

Before running REDHAWK you will need to set the OSSIEHOME and SDRROOT environment
variables.  This can be done in your ~/.bashrc.

    export OSSIEHOME=/usr/local/redhawk/core
    export SDRROOT=/var/redhawk/sdr
    export PYTHONPATH=${OSSIEHOME}/lib/python

## BULKIO and GPP

The bulkioInterfaces project requires minor patches (available on the
AxiosEngineering/framework-bulkioInterface repo) because of the way that Python
2.7 distutils interacts with the Makefile.  You can follow the same
"checkinstall" technique used for the core framework for both of these
packages.

    sudo checkinstall --provides=redhawk-bulkio --pkgversion=1.8.3 --pkgname=redhawk-bulkio make install

The GPP project works as-is (if you use ./reconf; ./configure; make), installing it is simple:

    sudo checkinstall --provides=redhawk-gpp --pkgversion=1.8.3 --pkgname=redhawk-gpp make install

## Configuring a domain

Unlike the RPM (and soon to be .DEB) you need to manually configure a domain.
The simply approach is: 

    sudo cp /var/redhawk/sdr/dom/domain/DomainManager.dmd.xml.template /var/redhawk/sdr/dom/domain/DomainManager.dmd.xml

Then replace @UUID@, @NAME@, and @DESCRIPTION@. 

Follow the tutorial to configure a DeviceManager node that contains a GPP device.

## Known Issues

1. The omnievent server has a bug in it that causes it to crash while running the
associated unittests (i.e. test_08_*).
