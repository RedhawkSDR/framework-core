#ifndef  _EVENT_APPENDER_CORBA_H
#define  _EVENT_APPENDER_CORBA_H
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <omniEvents/omniEvents.hh>


  namespace corba 
  {

    // naming service actions
    enum  NS_ACTION { NS_NOBIND=0, NS_BIND=1, NS_REBIND=2, NS_UNBIND=3 };

    struct Orb;

    typedef boost::shared_ptr< Orb >                OrbPtr;

    //
    // Orb
    //
    // Context for access to ORB and common CORBA services
    //
    struct Orb {

      // orb instantiation
      CORBA::ORB_var                          orb;

      // root POA for to handle object requests
      PortableServer::POA_var                 rootPOA;

      // use for activating objects
      PortableServer::POAManager_var          poaManager;

      // handle to naming service
      CosNaming::NamingContext_var            namingService;

      // handle to naming service
      CosNaming::NamingContextExt_var         namingServiceCtx;

      virtual ~Orb() {};

      //
      // establish singleton context for ORB ( rootPOA, root NamingService, root activator )
      //
      static OrbPtr Init( int argc, char **argv, const char *options[][2]=NULL, bool share=true );

      static OrbPtr Init();  // setup orb context using /etc/omniORB.cfg

      static OrbPtr Inst( ) { return _singleton; };

      static void   Terminate( bool forceShutdown = false );

    private:

      Orb() {};

      static OrbPtr   _singleton;

      static bool     _share;

    };

    //
    // Naming Methods
    //

    //
    // return a CosNaming sequence for the stringified name
    //
    CosNaming::Name *stringToName( std::string &name );

    //
    // return a CosNaming sequence for the stringified name
    //
    CosNaming::Name str2name(const char* namestr);


    //
    std::vector<std::string>  listRootContext();
    std::vector<std::string>  listContext( const CosNaming::NamingContext_ptr ctx, const std::string &dname="");


    int  CreateNamingContext( OrbPtr orb, const std::string &name );
    int  DeleteNamingContext( OrbPtr orb, const std::string &name );
    int  Bind( const std::string &name, CORBA::Object_ptr obj, CosNaming::NamingContext_ptr namingContext  );
    int  Bind( OrbPtr orb, const std::string &name,  CORBA::Object_ptr obj, const std::string &dir="" );

    int  Unbind( const std::string &name, CosNaming::NamingContext_ptr namingContext );
    int  Unbind( OrbPtr orb, const std::string &name, const std::string &namingContext="" );

    //
    //
    //  Event Channel Methods
    //
    //


    //
    // Look up the EventChannelFactory for the current ORB context
    //
    omniEvents::EventChannelFactory_ptr    GetEventChannelFactory( corba::OrbPtr &orb );
  
    //
    // GetEventChannel
    //
    // Get an EventChannel with in the provide ORB context, allow for creation if one does not exist
    //
    CosEventChannelAdmin::EventChannel_ptr GetEventChannel ( corba::OrbPtr &     orb, 
							     const std::string&  name, 
							     const bool          create=false,
							     const std::string   &host="localhost" );

    CosEventChannelAdmin::EventChannel_ptr GetEventChannel ( corba::OrbPtr &     orb, 
							     const std::string&  name, 
							     const std::string& ns_context, 
							     const bool          create=false,
							     const std::string   &host="localhost" );
    //
    // CreateEventChannel
    //
    // Create an EventChannel within the current ORB context, once created bind to the same name....
    //
    CosEventChannelAdmin::EventChannel_ptr CreateEventChannel( corba::OrbPtr &    orb, 
							       const std::string& name, 
							       NS_ACTION action=corba::NS_BIND );

    CosEventChannelAdmin::EventChannel_ptr CreateEventChannel( corba::OrbPtr &    orb, 
							       const std::string& name, 
							       const std::string& ns_context,
							       NS_ACTION action=corba::NS_BIND );

    //
    // Delete Event Channel
    //
    int DeleteEventChannel( corba::OrbPtr&       orb, 
			    const std::string&   name, 
			    NS_ACTION            action=NS_UNBIND);

    int DeleteEventChannel( corba::OrbPtr&       orb, 
			    const std::string&   name, 
			    const std::string&   nc_name, 
			    NS_ACTION            action=NS_UNBIND);


    //
    // PushEventSupplier
    //
    // This class will perform the publication portion of the a publisher/subscriber pattern 
    // over a CORBA EventChannel.  If the Channel does not exist it will try to create
    // and register the channel in the NamingService
    //
    //
    class  PushEventSupplier {

    public:    

      //
      //
      //
      class Supplier : virtual public POA_CosEventComm::PushSupplier {
    public:
      Supplier () {} ;
      virtual ~Supplier() {};
      virtual void disconnect_push_supplier () {} ;
    };


    //
    // Create the context for a PushEvent Supplier for a CORBA EventService
    //
    // @param orb reference to corba context and major services
    // @param channelName  event channel name to subscribe to or create
    // @param supplier actual supplier object that pushes information to event channel
    // @param retries number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    PushEventSupplier( corba::OrbPtr     &orb, 
		       const std::string &channelName,
		       Supplier          *supplier,
		       const int         retries=10, 
		       const int         retry_wait=10 );



    PushEventSupplier( corba::OrbPtr     &orb, 
		       const std::string &channelName,
		       const std::string &ncName,
		       Supplier          *supplier,
		       const int         retries=10, 
		       const int         retry_wait=10 );


    //
    // Create the context for a PushEvent Supplier for a CORBA EventService, uses internal Supplier object
    // to perform push event operation
    //
    // @param orb reference to corba context and major services
    // @param channelName  event channel name to subscribe to or create
    // @param retries number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    PushEventSupplier( corba::OrbPtr     &orb, 
		       const std::string &channelName,
		       const int         retries=10, 
		       const int         retry_wait=10 );


    PushEventSupplier( corba::OrbPtr     &orb, 
		       const std::string &channelName,
		       const std::string &ncName,
		       const int         retries=10, 
		       const int         retry_wait=10 );
    
    //
    //
    //
    virtual ~PushEventSupplier();

    //
    // Publish a CORBA Any object or a specific object subscribers.... 
    //
    template< typename T > 
      int     push( T &msg ) {
      CORBA::Any data;
      data <<= msg;
      proxy_for_consumer->push(data);
      return 0;
    }

    template< typename T > 
      int     push( T *msg ) {
      CORBA::Any data;
      data <<= msg;
      proxy_for_consumer->push(data);
      return 0;
    }

    int     push( CORBA::Any &data ) {
      proxy_for_consumer->push(data);
      return 0;
    }


    const Supplier *getSupplier() { return supplier; };

    protected:

    //
    // Channel name
    //
    std::string                                 name;

    //
    // Context where name is located
    //
    std::string                                 nc_name;

    //
    // handle to the EventChannel
    //
    CosEventChannelAdmin::EventChannel_var      channel;

    //
    // Get Supplier Admin interface 
    //
    CosEventChannelAdmin::SupplierAdmin_var     supplier_admin;

    //
    // Get proxy consumer 
    //
    CosEventChannelAdmin::ProxyPushConsumer_var proxy_for_consumer;

    //
    // Push Supplier...
    //
    Supplier                                      *supplier;

    //
    // number of retries to perform (-1 == try forever)
    //
    int retries;

    //
    // number of milliseconds to wait between retry operations
    //
    int retry_wait;


    private:
    void _init( corba::OrbPtr &orb );

    };

    //
    // PushEventConsumer
    //
    // This class will perform the subscription portion of the a publisher/subscriber pattern 
    // over a CORBA EventChannel.  If the Channel does not exist it will try to create
    // and register the channel in the NamingService
    //
    //
    class  PushEventConsumer {

    public:
      //
      // Callback interface used by BULKIO Ports when connect/disconnect event happens
      //
      typedef void   (*DataArrivedCallbackFn)( const CORBA::Any &data );

      //
      // Interface definition that will be notified when data arrives on a EventChannel
      //
      class DataArrivedListener {

      public:
	virtual void operator() ( const CORBA::Any &data ) = 0;
	virtual ~DataArrivedListener() {};

      };

      /**
       * Allow for member functions to receive connect/disconnect notifications
       */
      template <class T>
	class MemberDataArrivedListener : public DataArrivedListener
	{
	public:
	  typedef boost::shared_ptr< MemberDataArrivedListener< T > > SPtr;
      
	  typedef void (T::*MemberFn)( const CORBA::Any &data );

	  static SPtr Create( T &target, MemberFn func ){
	    return SPtr( new MemberDataArrivedListener(target, func ) );
	  };

	  virtual void operator() ( const CORBA::Any &data )
	  {
	    (target_.*func_)(data);
	  }

	  // Only allow PropertySet_impl to instantiate this class.
	  MemberDataArrivedListener ( T& target,  MemberFn func) :
	  target_(target),
	    func_(func)
	    {
	    }
	private:
	  T& target_;
	  MemberFn func_;
	};

      /**
       * Wrap Callback functions as ConnectionEventListener objects
       */
      class StaticDataArrivedListener : public DataArrivedListener
      {
      public:
	virtual void operator() ( const CORBA::Any &data )
	{
	  (*func_)(data);
	}

	StaticDataArrivedListener ( DataArrivedCallbackFn func) :
	func_(func)
	{
	}

      private:

	DataArrivedCallbackFn func_;
      };

      //
      // Define base class for consumers
      //

      typedef POA_CosEventComm::PushConsumer       Consumer;
      typedef CosEventComm::PushConsumer_var   Consumer_var;
      typedef CosEventComm::PushConsumer_ptr   Consumer_ptr;


      //
      // Create the context for a PushEvent Supplier for a CORBA EventService
      //
      // @param orb reference to corba context and major services
      // @param channelName  event channel name to subscribe to
      // @param consumer actual consumer object that receives pushed data
      // @param retries number of retries to perform when trying to establish subscriber interface (-1 tries forever)
      // @param retry_wait number of millisecs to wait between retries
      PushEventConsumer( corba::OrbPtr &    orb, 
			 const std::string &channelName, 
			 Consumer*          consumer,
			 const int          retries=10, 
			 const int          retry_wait=10 );

      PushEventConsumer( corba::OrbPtr &    orb, 
			 const std::string &channelName, 
			 const std::string &ncName, 
			 Consumer*          consumer,
			 const int          retries=10, 
			 const int          retry_wait=10 );

      PushEventConsumer( corba::OrbPtr &    orb, 
			 const std::string &channelName, 
			 const int          retries=10, 
			 const int          retry_wait=10 );

      PushEventConsumer( corba::OrbPtr &    orb, 
			 const std::string &channelName, 
			 const std::string &ncName, 
			 const int          retries=10, 
			 const int          retry_wait=10 );
      //
      // DTOR
      //
      virtual ~PushEventConsumer();

      //
      // 
      //
      const Consumer    *getConsumer() { return consumer; };

      // 
      // Attach/detach sequence does not work for some reason.
      //
#if 0    
      Consumer          *setConsumer( Consumer *newConsumer ) { 
	detach();
	consumer = newConsumer;
	attach();
      }

      void attach();
      void dettach();
#endif

      //
      // Attach callback listener when data arrives to Consumer object
      //
      template< typename T > inline
	void setDataArrivedListener(T &target, void (T::*func)( const CORBA::Any &data )  ) {
	dataArrivedCB =  boost::make_shared< MemberDataArrivedListener< T > >( boost::ref(target), func );
      };

      template< typename T > inline
	void setDataArrivedListener(T *target, void (T::*func)( const CORBA::Any &data  )  ) {
	dataArrivedCB =  boost::make_shared< MemberDataArrivedListener< T > >( boost::ref(*target), func );
      };

      void   setDataArrivedListener( DataArrivedListener *newListener );
      void   setDataArrivedListener( DataArrivedCallbackFn  newListener );

    protected:

      //
      // CallbackConsumer 
      //
      class CallbackConsumer : public Consumer {
    public:
      virtual ~CallbackConsumer() {};
      virtual void push( const CORBA::Any &data ) {
	if ( parent.dataArrivedCB ) {
	  try{
	    (*parent.dataArrivedCB)( data );
	  }
	  catch(...){
	  }
	}

      };
      virtual void disconnect_push_consumer () {} ;

    private:
      friend class PushEventConsumer;

      CallbackConsumer ( PushEventConsumer &parent) : 
      parent(parent) 
      {
      } ;

    protected:
      PushEventConsumer &parent;

    };

    friend class CallbackConsumer;

    //
    // Channel name
    //
    std::string                                 name;

    //
    // Naming context where channel is bound
    //
    std::string                                 nc_name;

    //
    // handle to the EventChannel
    //
    CosEventChannelAdmin::EventChannel_var      channel;

    //
    // Get Supplier Admin interface 
    //
    CosEventChannelAdmin::ConsumerAdmin_var     consumer_admin;

    //
    // Get proxy supplier that is providing the data
    //
    CosEventChannelAdmin::ProxyPushSupplier_var proxy_for_supplier;

    //
    // Push Consumer
    //
    Consumer                                    *consumer;

    //
    // PushConsumer Callback...
    // 
    // Used by default Consumer object to call registered callback
    //
    boost::shared_ptr< DataArrivedListener >    dataArrivedCB;

    //
    // number of retries to perform (-1 == try forever)
    //
    int retries;

    //
    // number of milliseconds to wait between retry operations
    //
    int retry_wait;

    private:
    void _init( corba::OrbPtr &orb );


    }; // end of PushEventConsumer

  };  // end of corba namespace


#endif
