
package org.ossie.events;

import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;
import org.omg.CORBA.*;
import org.omg.PortableServer.*;
import org.omg.CosEventChannelAdmin.*;
import org.omg.CosEventComm.*;
import org.omg.PortableServer.POA;
import org.apache.log4j.Logger;
import org.ossie.corba.utils.*;
import CF.EventChannelManagerPackage.*;

public class  Publisher  {

    public abstract class  Supplier extends PushSupplierPOA {

    };

  class DefaultSupplier extends Supplier {

    public DefaultSupplier ( Publisher inParent )  {
	parent=inParent;
    };

    public void disconnect_push_supplier () 
    {
	if ( parent != null ) parent.logger.debug( "DefaultSupplier: handler disconnect_push_supplier." );
	parent.proxy = null;
    };

    protected Publisher parent;
  };



    //
    //  Publisher for an Event Channel
    //
    // @param channel    event channel returned from the EventChannelManager
    // @param pub   interface that is notified when a disconnect occurs
    // @param retries    number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    public Publisher( EventChannel    channel )
	throws OperationNotAllowed
    {
	is_local = true;
	supplier = new DefaultSupplier(this);
	_init(channel,supplier);
    }

    public Publisher( EventChannel       channel ,
		      Supplier           supplier ) 
	throws OperationNotAllowed
    {
	is_local = false;
        _init( channel, supplier );
    }


    public void  terminate() {
        logger.debug("TERMINATE - START." );

        // stop our supplier from receiving data...
        if ( is_local )  {
            org.ossie.corba.utils.deactivateObject(supplier);
        }

        // disconnect the channel
        disconnect();

        // free up the resource
        proxy = null;
        supplier=null;

        logger.debug("TERMINATE - END." );
    }


    public int disconnect() {
        return this.disconnect(10,10);
    }

    public int disconnect( int retries, int retry_wait ) {

        int retval=-1;
        int tries = retries;
        if ( proxy != null ) { 
            do {
                try {
                    proxy.disconnect_push_consumer();
                    retval=0;
                    break;
                }
                catch (COMM_FAILURE ex) {
                    logger.error( "Caught COMM_FAILURE Exception disconnecting Push Supplier! Retrying..." );
                }
                if ( retry_wait > 0 ) {
                    try {
                        java.lang.Thread.sleep(retry_wait*1000);
                    } catch (final InterruptedException ex) {
                    }
                }
                tries--;
            } while(tries>0);
        }

        logger.debug( "Publisher disconnected ...." );     
        return retval;
    }


    public int connect() {
        return this.connect(10,10);
    }

    public int connect( int retries, int retry_wait ) {

        int retval=-1;

        if ( channel == null ) {
            return retval;
        }

	int tries=retries;
        if ( proxy == null ) {
            SupplierAdmin admin=null;
            do {
                try {
                    admin = channel.for_suppliers ();
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
            } while ( tries > 0);
	
            if ( admin == null ) return retval;

            tries=retries;
            do {
                try {
                    proxy = admin.obtain_push_consumer ();
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
        }

        if ( proxy == null ) return retval;

	PushSupplier   sptr=null;
	if ( supplier != null ) {
            org.ossie.corba.utils.activateObject(supplier, null);
	    sptr = supplier._this();
	}

        // now attach supplier to the proxy
        tries=retries;
        do {
            try {
                proxy.connect_push_supplier(sptr);
                retval=0;
                break;
            }
            catch(BAD_PARAM ex) {
                logger.error( "Caught BAD_PARAM exception connecting Push Supplier!");
                break;
            }
            catch(AlreadyConnected ex) {
                retval=0;
                logger.error("Proxy Push Consumer already connected!");
                break;
            }
            catch(COMM_FAILURE ex) {
                logger.error("Caught COMM_FAILURE exception "
                        +"connecting Push Supplier! Retrying...");
            }
            if ( retry_wait > 0 ) {
                try {
                    java.lang.Thread.sleep(retry_wait*1000);
                } catch (final InterruptedException ex) {
                }
            }
            tries--;
        } while ( tries > 0 );

        return retval;
    };


    public int    push( Any data ) {
      int retval=0;
      try {
        if (  proxy != null ) {
            proxy.push(data);
        }
        else{
          retval=-1;
        }
      }
      catch( Exception ex) {
        retval=-1;
      }

      return retval;
    }



    // handle to the Event Channel ... duplicated channel so we own this object 
    protected EventChannel              channel;

    // handle to object that publishes the event to the channel's consumers
    //protected ProxyPushConsumerOperations   proxy;
    protected ProxyPushConsumer         proxy = null;

    // handle to object that responds to disconnect messages
    protected Supplier                  supplier = null;

    //
    // designator if supplier is a local object or provided
    //
    protected boolean                   is_local=true;

    //
    // logger
    protected Logger                    logger=null;



    private void _init( EventChannel inChannel, Supplier inSupplier ) 
        throws OperationNotAllowed {

	logger = Logger.getLogger("ossie.events.Publisher");

        // create a local supplier object 
        supplier = inSupplier;

        // if user passes a bad param then throw...
        channel = inChannel;
	if ( inChannel == null ) throw new OperationNotAllowed();

        // initialize the event channel for a publisher and the local supplier interface
        connect( );
    }

};
