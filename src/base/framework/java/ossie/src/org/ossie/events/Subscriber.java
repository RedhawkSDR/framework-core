package org.ossie.events;

import java.util.Queue;
import java.util.LinkedList;
import org.apache.log4j.Logger;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;
import org.omg.CORBA.*;
import org.omg.CosEventComm.*;
import org.omg.CosEventChannelAdmin.*;
import CF.EventChannelManagerPackage.*;
import org.ossie.properties.AnyUtils;


public class  Subscriber  {

    public abstract class EventSubscriberConsumer extends PushConsumerPOA {
    };

    public interface  DataArrivedListener  {
	public void processData( final Any data );
    };


    private class DefaultConsumer extends EventSubscriberConsumer {

	public DefaultConsumer ( Subscriber inParent ) {
	    parent = inParent;
	}

	public void disconnect_push_consumer() {
	    if (parent != null) {
		parent.logger.debug( "handle disconnect_push_consumer." );
		if ( parent.proxy != null ){
		    parent.proxy = null;
		}
	    }
	}
    
    
	public  void push( final org.omg.CORBA.Any data ) {
	    if ( parent != null ) {
		// if parent defines a callback
		if ( parent.dataArrivedCB != null ) {
                    // RESOLVE need to allow for Generic callback method to convert data and
                    // push data...
		    try{
                        parent.dataArrivedCB.processData( data );
		    }
		    catch( Throwable e){
		    }
		}
		else {
		    parent.events.add(data);
		}
	    };
	};


	private Subscriber         parent;
  };



    //
    //  Subscriber for an Event Channel
    //
    // @param channel    event channel returned from the EventChannelManager
    // @param pub   interface that is notified when a disconnect occurs
    // @param retries    number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    public Subscriber( EventChannel       inChannel ) 
	throws OperationNotAllowed
    {
	this(inChannel,null );
    }

    public Subscriber( EventChannel            inChannel,
		       DataArrivedListener     newListener )
	throws OperationNotAllowed
    {
        _init( inChannel, newListener );
    }


    public int getData( Any ret ) {

      int retval=-1;
      try{

        // check if callback method is enable.. it so then return
        if ( dataArrivedCB != null ) return retval;

        // check if data is available
        if ( events.size() < 1 ) return retval;
          
        Any  rawdata =  events.remove();
          
        ret = rawdata;
        retval=0;
      }
      catch( Throwable e) {
      }

      return retval;
    }


    public void terminate () {

        logger.trace("Subscriber: TERMINATE START" );

        if ( consumer != null ) {
            org.ossie.corba.utils.deactivateObject(consumer);
        }

        disconnect();

        proxy = null;
        consumer = null;

        logger.trace("Subscriber: TERMINATE END." );
    }


    public int disconnect() {
        return this.disconnect(10,10);
    }

    public int disconnect( int retries, int retry_wait ) {

        logger.debug("Subscriber: disconnect ....." );
        int retval = -1;
        int tries = retries;
        if ( proxy != null ) {
            do {
                try {
                    logger.debug("Subscriber: disconnect_push_supplier... ");
                    proxy.disconnect_push_supplier();
                    retval=0;
                    break;
                }
                catch (COMM_FAILURE ex) {
                    logger.warn("Caught COMM_FAILURE Exception,  diconnecting Push Consumer for Subscriber,  Retrying..." );
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                }
                tries--;
            } while(tries>0);
            logger.debug("Subscriber: ProxyPushSupplier disconnected." );

        }
        
        return retval;
    }

    public int connect() {
        return this.connect(10,10);
    }

    public int connect( int retries, int retry_wait ) {

        int retval=-1;
	int tries=retries;

	if ( channel == null ) {
	    logger.warn( "Subscriber,  Channel resource not available ");
	    return retval;
	}

        if ( proxy == null ) {
            ConsumerAdmin         admin=null;
            do {
                try {
                    logger.debug( "Subscriber,  Grab admin object...");
                    admin = channel.for_consumers ();
                    break;
                }
                catch (COMM_FAILURE ex) {
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }

                } 
                tries--;
            } while ( tries > 0 );


            if ( admin == null  ) return retval;

            logger.debug( "Subscriber,  Obtained ConsumerAdmin...");
            tries=retries;
            do {
                try {
                    logger.debug( "Subscriber,  Grab push supplier proxy.");
                    proxy = admin.obtain_push_supplier();
                    break;
                }
                catch (COMM_FAILURE ex) {
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                }
                tries--;
            } while ( tries > 0 );

            logger.debug( "Subscriber,  Obtained ProxyPushConsumer." );
        }

	if ( proxy == null  ) return retval;

        PushConsumer sptr;
	if  ( consumer == null ) {
            logger.debug( "Subscriber, Create local DefaultConsumer for EventChannel Subscriber." );
	    consumer = new DefaultConsumer( this );
	    if ( consumer == null ) return retval;
            org.ossie.corba.utils.activateObject(consumer, null);
	    sptr = consumer._this();
	}
	else {
            org.ossie.corba.utils.activateObject(consumer, null);
	    sptr = consumer._this();
	}

	// now attach supplier to the proxy
        tries=retries;
	do {
	    try {
		proxy.connect_push_consumer( sptr );
                logger.debug( "Subscriber, Connected Consumer to EventChannel....." );
                retval=0;
		break;
	    }
	    catch(TypeError ex) {
                logger.error( "Subscriber, Caught TypeError exception connecting Push Consumer!");
		break;
	    }
	    catch(BAD_PARAM ex) {
                logger.error( "Subscriber, Caught BAD_PARAM exception connecting Push Consumer!");
		break;
	    }
	    catch(AlreadyConnected ex) {
                logger.warn( "Subscriber, Push Consumer already connected!");
                retval=0;
		break;
	    }
	    catch(COMM_FAILURE ex) {
                logger.error( "Subscriber, Caught COMM_FAILURE exception "
				   +"connecting Push Supplier! Retrying...");
	    }
	    if ( retry_wait > 0 ) {
               try {
		   java.lang.Thread.sleep(retry_wait*1000);
                } catch (final InterruptedException ex) {
	       }

	    }
	    tries--;
	} while ( tries > 0);


        return retval;

    };


    // handle to the Event Channel ... duplicate the channel so we own this object 
    protected EventChannel                  channel;

    // handle to object that publishes the event to the channel's consumers
    //protected ProxyPushSupplierOperations   proxy;
    protected ProxyPushSupplier             proxy;

    // handle to object that responds to disconnect messages
    protected EventSubscriberConsumer       consumer;

    //
    // Logger object 
    //
    protected Logger                        logger=null;

    //
    // Callback interface when event messages arrive
    //
    protected DataArrivedListener           dataArrivedCB=null;

    
    protected Queue< Any >                  events=null;


    private  void _init( EventChannel inChannel, DataArrivedListener newListener ) 
        throws OperationNotAllowed 
    {
	logger = Logger.getLogger("ossie.events.Subscriber");

        dataArrivedCB = newListener;

        // create a local consumer object for the event channel
        consumer = new DefaultConsumer(this);

        // create queue to hold events
        events = new LinkedList< Any >();

        // if user passes a bad param then throw...
        channel=inChannel;
	if ( inChannel == null ) throw new OperationNotAllowed();

	// connect to the event channel for a subscriber pattern
        connect();
    }



};
