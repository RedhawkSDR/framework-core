# REDHAWK Core Framework Ubuntu Instructions

## Installing REDHAWK

## Configuring your terminal to load the REDHAWK environment

Per REDHAWK conventions certian environment variables need to be defined.  When
you install REDHAWK it creates scripts that will set these up for you, but by default
gnome-terminal sessions will not correctly source them.

To have gnome-terminal start bash as a login shell do the following:

1. start a gnome-terminal.
2. go to Edit -> Profile Preferences -> Title and Command
3. enable "Run Command as a login shell".
4. restart the gnome-terminal.

Bash should now start as a login shell and thus will read /etc/profile.d.
Alternatively, you can add the following lines to your ~/.bashrc

    . /etc/profile.d/redhawk.sh
    . /etc/profile.d/redhawk-sdrroot.sh

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

    export JAVA_HOME /usr/lib/jvm/java-6-openjdk-amd64

You may prefer to set this in your ~/.bashrc or equivalent.


## Configuring your system to support building components/devices

If you plan on doing building REDHAWK components or devices, you will want to
prepare your system with the proper build tools and libraries:

    sudo apt-get install build-essential \
                         libboost-dev \
                         libboost-system-dev \
                         libboost-filesystem-dev \
                         libboost-regex-dev \
                         libboost-thread-dev \
                         libcos4-dev \
                         libomnievents-dev \
                         libomniorb4-dev \
                         liblog4cxx10-dev

## Known Issues

1. The omnievent server has a bug in it that causes it to crash while running the
associated unittests (i.e. test_08_*).

2. Ubuntu uses 'dash' for /bin/sh while many of the REDHAWK build scripts expect /bin/sh to
be 'bash'.  You have two choices:

   sudo dpkg-reconfigure dash # Answer no

This will make bash your /bin/sh shell; this may make your system boot ever so slightly slower.

3. Ubuntu places the omniidl library outside of the Python Path.  When using the IDE code
generators this must be importable.  You have a a few choices to fix this issue:

Alternativly, you can symlink omniidl into the dist-packages folder:

   sudo ln -s /usr/lib/omniidl/omniidl /usr/lib/python2.7/dist-packages/omniidl

Finally, you can manually add /usr/lib/omniidl/omniidl to your PyDev interpreter configuration.

## Building from source

    cd src                     
    ./reconf
    ./configure --with-ossie=/usr/local/redhawk/core --with-sdr=/var/redhawk/sdr
    make

Installation can be performed by running:

    sudo make install

However, you may prefer to use the checkinstall tool so that you can easily uninstall.

    sudo checkinstall --provides=redhawk --pkgversion=1.8.3 --pkgname=redhawk make install

Before running REDHAWK you will need to set the OSSIEHOME and SDRROOT environment
variables.  This can be done in your ~/.bashrc.

    export OSSIEHOME=/usr/local/redhawk/core
    export SDRROOT=/var/redhawk/sdr
    export PYTHONPATH=${OSSIEHOME}/lib/python
    sudo checkinstall --provides=redhawk-bulkio --pkgversion=1.8.3 --pkgname=redhawk-bulkio make install

The GPP project works as-is (if you use ./reconf; ./configure; make), installing it is simple:

    sudo checkinstall --provides=redhawk-gpp --pkgversion=1.8.3 --pkgname=redhawk-gpp make install

## Configuring a domain

Unlike the RPM (and soon to be .DEB) you need to manually configure a domain.
The simply approach is: 

    sudo cp /var/redhawk/sdr/dom/domain/DomainManager.dmd.xml.template /var/redhawk/sdr/dom/domain/DomainManager.dmd.xml

Then replace @UUID@, @NAME@, and @DESCRIPTION@. 

Follow the tutorial to configure a DeviceManager node that contains a GPP device.
