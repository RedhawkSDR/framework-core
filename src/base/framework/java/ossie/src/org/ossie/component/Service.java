/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/**
 *
 * Identification: $Revision$
 */
package org.ossie.component;

import java.io.File;
import java.io.FileOutputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.Appender;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Level;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.PropertyConfigurator;
import org.omg.CORBA.ORB;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAHelper;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import CF.DeviceManager;
import CF.DeviceManagerHelper;
import CF.InvalidObjectReference;

public abstract class Service
{
    public final static Logger logger = Logger.getLogger(Service.class.getName());
    
    /** The CORBA service object */
    protected org.omg.CORBA.Object serviceObj = null;
    /** The Device Manager the service is registered with */
    protected DeviceManager devMgr = null;
    /** The service name */
    protected String serviceName;

    public Service()
    {
        this.serviceName = "";
    }
    
    /**
     * Parse the set of SCA execparam arguments into a Map
     * 
     * @param args as passed in from the command line
     * @return a map of arg/value pairs
     */
    protected static Map<String, String> parseArgs(String[] args)
    {
        logger.trace("Starting with execparams: " + Arrays.toString(args));
        Map<String, String> result = new Hashtable<String, String>();
        for (int i=0; i < (args.length - 1); i = i +2) {
            result.put(args[i], args[i+1]);
        }
        logger.trace("Starting with execparams: " + result);
        return result;
    }

    /**
     * This function returns the filename of the local logging configuration file given an SCA FileSystem logging file.
     * 
     * @param uri string containing SCA File System filename for logging configuration file
     * @return the string representing the local logging configuration filename
     */
    public static String getLogConfig(String uri, org.omg.CORBA.ORB orb)
    {
        String localPath = "";

        int fsPos = uri.indexOf("?fs=");
        if (fsPos == -1) {
            return localPath;
        }

        String IOR = uri.substring(fsPos + 4);
        org.omg.CORBA.Object obj = orb.string_to_object(IOR);
        if (obj == null) {
            return localPath;
        }

        CF.FileSystem fileSystem = CF.FileSystemHelper.narrow(obj);
        if (fileSystem == null) {
            return localPath;
        }

        String remotePath = uri.substring(0, fsPos);
        CF.OctetSequenceHolder data = new CF.OctetSequenceHolder ();
        try {
            CF.File remoteFile = fileSystem.open(remotePath, true);
            int size = remoteFile.sizeOf();
            remoteFile.read(data, size);

            String tempPath = remotePath;
            int slashPos = remotePath.lastIndexOf('/');
            if (slashPos != -1) {
                tempPath = tempPath.substring(slashPos + 1);
            }
            
            FileOutputStream localFile = new FileOutputStream(tempPath);
            localFile.write(data.value);
            localPath = tempPath;
            localFile.close();
            return localPath;
        } catch (Exception e){
            return localPath;
        }
    }
    
    abstract protected Servant newServant(final POA p);
    abstract public void terminateService(); 
    
    public static void start_service(final Class<? extends Service> clazz, final String[] args, final boolean builtInORB, final int fragSize, final int bufSize) 
    throws InstantiationException, IllegalAccessException, InvalidObjectReference, IllegalArgumentException, SecurityException, InvocationTargetException, NoSuchMethodException 
    {
        final Properties props = new Properties();
        if (!builtInORB) {
            props.put("org.omg.CORBA.ORBClass", "org.jacorb.orb.ORB");
            props.put("org.omg.CORBA.ORBSingletonClass", "org.jacorb.orb.ORBSingleton");
            props.put("jacorb.fragment_size", Integer.toString(fragSize));
            props.put("jacorb.outbuf_size", Integer.toString(bufSize));
            props.put("jacorb.maxManagedBufSize", "23");
        } else {
            props.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
            props.put("com.sun.CORBA.giop.ORBBufferSize", Integer.toString(bufSize));
        }
        start_service(clazz, args, props);
    }
    
    public static void start_service(final Class<? extends Service> clazz, final String[] args, final Properties props) 
    throws InstantiationException, IllegalAccessException, InvalidObjectReference, IllegalArgumentException, SecurityException, InvocationTargetException, NoSuchMethodException
    {
        final org.omg.CORBA.ORB orb = ORB.init((String[]) null, props);

        // get reference to RootPOA & activate the POAManager
        POA rootpoa = null;
        try {
            final org.omg.CORBA.Object poa = orb.resolve_initial_references("RootPOA");
            rootpoa = POAHelper.narrow(poa);
            rootpoa.the_POAManager().activate();
        } catch (final AdapterInactive e) {
            // PASS
        } catch (final InvalidName e) {
            // PASS
        }

        Map<String, String> execparams = parseArgs(args);

        // Sets up the logging
        String loggingConfigURI = null;
        if (execparams.containsKey("LOGGING_CONFIG_URI")) {
            loggingConfigURI = execparams.get("LOGGING_CONFIG_URI");
            if (loggingConfigURI.indexOf("file://") != -1){
                int startIndex = loggingConfigURI.indexOf("file://") + 7;
                PropertyConfigurator.configure(loggingConfigURI.substring(startIndex));
            }else if (loggingConfigURI.indexOf("sca:") != -1){
                int startIndex = loggingConfigURI.indexOf("sca:") + 4;
                String localFile = getLogConfig(loggingConfigURI.substring(startIndex), orb);
                File testLocalFile = new File(localFile);
                if (localFile.length() > 0 && testLocalFile.exists()){
                    PropertyConfigurator.configure(localFile);
                }
            }
        } else {
            // If no logging config file, then set up logging using DEBUG_LEVEL exec param
            int debugLevel = 3; // Default level is INFO
            if (execparams.containsKey("DEBUG_LEVEL")) {
                debugLevel = Integer.parseInt(execparams.get("DEBUG_LEVEL"));
            }
            LogManager.getLoggerRepository().resetConfiguration();
            Logger root = Logger.getRootLogger();
            Layout layout = new PatternLayout("%p:%c - %m%n");
            Appender appender = new ConsoleAppender(layout);
            root.addAppender(appender);
            if (debugLevel == 0) {
                root.setLevel(Level.FATAL);
            } else if (debugLevel == 1) {
                root.setLevel(Level.ERROR);
            } else if (debugLevel == 2) {
                root.setLevel(Level.WARN);
            } else if (debugLevel == 3) {
                root.setLevel(Level.INFO);
            } else if (debugLevel == 4) {
                root.setLevel(Level.DEBUG);
            } else if (debugLevel >= 5) {
                root.setLevel(Level.ALL);
            }
        }
        
        DeviceManager deviceMgr = null;
        if (execparams.containsKey("DEVICE_MGR_IOR")) {
            deviceMgr = DeviceManagerHelper.narrow(orb.string_to_object(execparams.get("DEVICE_MGR_IOR")));
        }

        if (!execparams.containsKey("SERVICE_NAME")) {
            logger.warn("No 'SERVICE_NAME' argument provided");
            execparams.put("SERVICE_NAME", "");
        }
        
        final Service service_i = clazz.getConstructor(Map.class).newInstance(execparams);
        Servant tie = service_i.newServant(rootpoa);
        tie._this_object(orb);
        service_i.devMgr = deviceMgr;
        
        if (service_i.devMgr != null) {
            logger.debug("Registering service with device manager");
            try {
                service_i.serviceObj = rootpoa.servant_to_reference(tie);
                service_i.devMgr.registerService(service_i.serviceObj, execparams.get("SERVICE_NAME"));
            } catch (ServantNotActive e) {
                e.printStackTrace();
            } catch (WrongPolicy e) {
                e.printStackTrace();
            }
        }

        // Adds a shutdown hook for signals sent from the Device Manager
        final Map<String, String> execparamsCopy = execparams;
        Runtime.getRuntime().addShutdownHook(new Thread() {
            public void run() {
                if (service_i.devMgr != null) {
                    try {
                        service_i.devMgr.unregisterService(service_i.serviceObj, execparamsCopy.get("SERVICE_NAME"));
                        service_i.terminateService();
                    } catch (InvalidObjectReference e) {
                        logger.info("Error while unregistering service");
                    }
                }
                orb.destroy();
            }
        });
        
        logger.debug("Starting ORB event loop");      
        orb.run();
    }
}
