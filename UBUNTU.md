# REDHAWK Core Framework Ubuntu Instructions
## Configuring your terminal to load the REDHAWK environment

## Configuring your system to support building components/devices

If you plan on doing building REDHAWK components or devices, you will want to
prepare your system with the proper build tools and libraries: 

##NOTE: If running ubuntu 13.10 the default boost installed will be 1.53 (boost filesystem v3). 
##      The default installation location for boost is not in the default PATH. 
##      When running configure you will need to specify the new boost install location.
##      To install boost 1.49( boost filesystem v2) specify libboost1.49 instead of libboost

    sudo apt-get install -y build-essential \
                            openmpi-bin \
                            libopenmpi-dev \
                            python2.7 \
                            python2.7-dev \
                            uuid \
                            uuid-dev \
                            openjdk-7-jdk \
                            libtool \
                            autotools-dev \
                            autoconf \
                            automake \
                            python-omniorb \
                            omnievents \
                            omniidl \
                            omniidl-python \
                            omniorb \
                            omniorb-idl \
                            omniorb-nameserver \
                            libcos4-dev \
                            libomnievents-dev \
                            libomniorb4-dev \
                            python-numpy \
                            python-omniorb \
                            liblog4cxx10-dev \
                            xsdcxx \
                            libboost-dev \
                            libboost-system-dev \
                            libboost-filesystem-dev \
                            libboost-regex-dev \
                            libboost-thread-dev \
                            vim

To create a writable location for the compiled REDHAWK source, enter the following commands.	
    mkdir -p  /var/lib/redhawk/core
    mkdir -p  /var/lib/redhawk/sdr

Per REDHAWK conventions certian environment variables need to be defined.  When
you install REDHAWK it creates scripts that will set these up for you, but by default
gnome-terminal sessions will not correctly source them.

To have gnome-terminal start bash as a login shell do the following:

1. start a gnome-terminal.
2. go to Edit -> Profile Preferences -> Title and Command
3. enable "Run Command as a login shell".
4. restart the gnome-terminal.

##Check out the Source Code from github.com
   mkdir git
   cd git
   git clone https://github.com/RedhawkSDR/framework-core
   cd framework-core/src

## Building the core framework from source
## --with-ossie becomes the path for OSSIEHOME
## --with-sdr becomes the path for SDRROOT
## --sysconfdir established the location for /etc/profile.d and ld.so
## if you installed libboost1.1.49 not libboost remove the --with-boost-libdir=/usr/lib/x86_64-linux-gnu  
    cd src                     
    ./reconf
    ./configure --with-boost-libdir=/usr/lib/x86_64-linux-gnu --with-ossie=/usr/local/redhawk/core --with-sdr=/var/redhawk/sdr --sysconfdir=/etc
    make

Installation can be performed by running:

    sudo make install

However, you may prefer to use the checkinstall tool so that you can easily uninstall.

    sudo checkinstall --provides=redhawk --pkgversion=1.9.0 --pkgname=redhawk make install

   cd ../..

#Build the GPP 
   git clone https://github.com/RedhawkSDR/framework-GPP.git
   cd framework-gpp/python
   ./reconf;./configure;make;make install
   sudo checkinstall --provides=redhawk-gpp --pkgversion=1.9.0 --pkgname=redhawk-gpp make install
   cd ../..

#Build the bulkioInterfaces 
   git clone https://github.com/RedhawkSDR/framework-bulkioInterfaces.git
   cd framework-bulkioInterfaces
   ./reconf;./configure;make;make install
   sudo checkinstall --provides=redhawk-bulkioInterfaces --pkgversion=1.9.0 --pkgname=redhawk-bulkioInterfaces make install
   cd ../../

#Build the code generators  
   git clone https://github.com/RedhawkSDR/framework-codegen.git
   cd framework-codegen
   python setup.py install
   sudo checkinstall --provides=redhawk-codegen --pkgversion=1.9.0 --pkgname=redhawk-codegen make install
   cd ..

Before running REDHAWK you will need to set the OSSIEHOME and SDRROOT environment
Add the following to .bashrc to set OSSIEHOME, SDRROOT, PYTHONPATH, JAVA_HOME, and PATH.
    echo "export OSSIEHOME=/var/lib/redhawk/core" >> ~/.bashrc
    echo "export SDRROOT=/var/lib/redhawk/sdr" >> ~/.bashrc
    echo "export PYTHONPATH=\${OSSIEHOME}/lib/python" >> ~/.bashrc
    echo "export JAVA_HOME=/usr/bin/java" >> ~/.bashrc
    echo "export PATH=\${OSSIEHOME}/bin:\{$JAVA_HOME}/bin:\$PATH" >> ~/.bashrc
    source ~/.bashrc    
    
## Configuring a domain

Requires a manually configured domain.
The simply approach is: 

    sudo cp /var/redhawk/sdr/dom/domain/DomainManager.dmd.xml.template /var/redhawk/sdr/dom/domain/DomainManager.dmd.xml

Then replace @UUID@, @NAME@, and @DESCRIPTION@. 

Follow the tutorial to configure a DeviceManager node that contains a GPP device.

## Setting up /etc/omniORB.cfg

The default Ubuntu configuration needs modifications to work with the baseline
REDHAWK framework.

1. Increase the giopMaxMsgSize to 10485760 (i.e. 10MB)
2. Comment out the line "DefaultInitRef = corbaloc::"
3. Add the line "InitRef = NameService=corbaname::localhost"
4. Add the line "EventService=corbaloc::localhost:11169/omniEvents"

## Setup JAVA_HOME

The REDHAWK runtime and build process expect the JAVA_HOME environment variable
to be set.  Set this to a JDK installed in /usr/lib/jvm. For example:

    export JAVA_HOME /usr/lib/jvm/java-7-openjdk-amd64

## Known Issues

1. Ubuntu uses 'dash' for /bin/sh while many of the REDHAWK build scripts expect /bin/sh to
be 'bash'.  You have two choices:

   sudo dpkg-reconfigure dash # Answer no

This will make bash your /bin/sh shell; this may make your system boot ever so slightly slower.

2. Ubuntu places the omniidl library outside of the Python Path.  When using the IDE code
generators this must be importable.  You have a a few choices to fix this issue:

Alternativly, you can symlink omniidl into the dist-packages folder:

   sudo ln -s /usr/lib/omniidl/omniidl /usr/lib/python2.7/dist-packages/omniidl

Finally, you can manually add /usr/lib/omniidl/omniidl to your PyDev interpreter configuration.

