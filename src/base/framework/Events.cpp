#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ossie/RedhawkDefs.h>
#include <ossie/CF/StandardEvent.h>
#include <ossie/Events.h>
#include <ossie/Resource_impl.h>
#include <ossie/Device_impl.h>


REDHAWK_CPP_NAMESPACE_BEGIN


namespace events {


  DomainEventWriter::DomainEventWriter( PublisherPtr pub ) : pub(pub) {};


  void DomainEventWriter::sendComponentTermination( const ComponentTerminationEvent  &evt ) {
      if (pub) {
        StandardEvent::AbnormalComponentTerminationEventType new_message;
        new_message.deviceId = CORBA::string_dup(evt.device_id.c_str());
        new_message.componentId = CORBA::string_dup(evt.component_id.c_str());
        new_message.applicationId = CORBA::string_dup(evt.application_id.c_str());
        try {
          RH_NL_DEBUG("DomainEventWriter", "SEND ComponentTerminationEvent device id:"<<evt.device_id<<" application id:"<<evt.application_id<<" component id:"<<evt.component_id);
          pub->push(new_message);
        } catch ( ... ) {
          RH_NL_WARN("DomainEventWriter", "Unable to send the ComponentTerminationEvent, (comm error) device id:"<<evt.device_id<<" application id:"<<evt.application_id<<" component id:"<<evt.component_id);
        }
      }
      else {
          RH_NL_WARN("DomainEventWriter", "Unable to send the ComponentTerminationEvent, (no publisher available) device id:"<<evt.device_id<<" application id:"<<evt.application_id<<" component id:"<<evt.component_id);
      }
  };

  void DomainEventWriter::sendComponentTermination( const char* deviceId,
                                                    const char* applicationId,
                                                    const char *componentId )
  {
    ComponentTerminationEvent evt;
    evt.device_id = deviceId;
    evt.application_id = applicationId;
    evt.component_id = componentId;
    sendComponentTermination(evt);
  }


  void DomainEventWriter::sendObjectStateChange( const char* producerId, 
						 const char* sourceId,
						 StandardEvent::StateChangeCategoryType stateChangeCategory, 
						 StandardEvent::StateChangeType stateChangeFrom, 
						 StandardEvent::StateChangeType stateChangeTo )
  {
    ObjectStateChangeEvent evt;
    evt.prod_id = producerId;
    evt.source_id = sourceId;
    evt.category = stateChangeCategory;
    evt.from  = stateChangeFrom;
    evt.to = stateChangeTo;
    sendObjectStateChange(evt);
  }


  void DomainEventWriter::sendObjectStateChange( const ObjectStateChangeEvent &evt ) {
    if (pub) {
      StandardEvent::StateChangeEventType new_message;
      new_message.producerId = CORBA::string_dup(evt.prod_id.c_str());
      new_message.sourceId = CORBA::string_dup(evt.source_id.c_str());
      new_message.stateChangeCategory = evt.category;
      new_message.stateChangeFrom = evt.from;
      new_message.stateChangeTo = evt.to;
      try {
	RH_NL_DEBUG("DomainEventWriter", "SEND StateChangeEvent:  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" state change category:"<<evt.category<<" state change from/to: "<<evt.from<<"/"<<evt.to);
	pub->push(new_message);
	RH_NL_DEBUG("DomainEventWriter", "SEND StateChangeEvent RETURNING....");  
      } catch ( ... ) {
	RH_NL_WARN("DomainEventWriter", "Unable to send the following StateChangeEvent (the event service might not be running):\n  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" state change category:"<<evt.category<<" state change from/to: "<<evt.from<<"/"<<evt.to);
      }
    }
    else {
	RH_NL_WARN("DomainEventWriter", "Unable to send the following StateChangeEvent (no publisher available):\n  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" state change category:"<<evt.category<<" state change from/to: "<<evt.from<<"/"<<evt.to);
    }
  };


  void DomainEventWriter::sendResourceStateChange( const char* sourceId, 
						 const char* sourceName,
						 ExtendedEvent::ResourceStateChangeType stateChangeFrom, 
						 ExtendedEvent::ResourceStateChangeType stateChangeTo )
  {
    ResourceStateChangeEvent evt;
    evt.source_id = sourceId;
    evt.source_name = sourceName;
    evt.from  = stateChangeFrom;
    evt.to = stateChangeTo;
    sendResourceStateChange(evt);
  }


  void DomainEventWriter::sendResourceStateChange( const ResourceStateChangeEvent &evt ) {
    if (pub) {
      ExtendedEvent::ResourceStateChangeEventType new_message;
      new_message.sourceId = CORBA::string_dup(evt.source_id.c_str());
      new_message.sourceName = CORBA::string_dup(evt.source_name.c_str());
      new_message.stateChangeFrom = evt.from;
      new_message.stateChangeTo = evt.to;
      try {
        RH_NL_DEBUG("DomainEventWriter", "SEND ResourceStateChangeEvent:  source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" state change from/to: "<<evt.from<<"/"<<evt.to);
        pub->push(new_message);
        RH_NL_DEBUG("DomainEventWriter", "SEND ResourceStateChangeEvent RETURNING....");  
      } catch ( ... ) {
        RH_NL_WARN("DomainEventWriter", "Unable to send the following ResourceStateChangeEvent (the event service might not be running):\n  source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" state change from/to: "<<evt.from<<"/"<<evt.to);
      }
    } else {
        RH_NL_WARN("DomainEventWriter", "Unable to send the following ResourceStateChangeEvent (no publisher available):\n  source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" state change from/to: "<<evt.from<<"/"<<evt.to);
    }
  };

  void DomainEventWriter::sendAddedEvent( const char* producerId, 
					const char* sourceId, 
					const char* sourceName, 
					RH_Object src_obj,
					StandardEvent::SourceCategoryType sourceCategory ) {
    DomainStateEvent  evt;
    evt.prod_id = producerId;
    evt.source_id = sourceId;
    evt.source_name = sourceName;
    evt.category = sourceCategory;
    evt.obj = src_obj;
    sendAddedEvent(evt);
    
  }

  void DomainEventWriter::sendAddedEvent( const DomainStateEvent  &evt )  {

      if (pub) {
        StandardEvent::DomainManagementObjectAddedEventType new_message;
        new_message.producerId = CORBA::string_dup(evt.prod_id.c_str());
        new_message.sourceId = CORBA::string_dup(evt.source_id.c_str());
        new_message.sourceName = CORBA::string_dup(evt.source_name.c_str());
	new_message.sourceIOR = CORBA::Object::_duplicate(evt.obj);
        new_message.sourceCategory = evt.category;
        try {
          RH_NL_DEBUG("DomainEventWriter", "SEND ObjectAddedEvent producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" source category: "<<evt.category);
          pub->push(new_message);
          RH_NL_DEBUG("DomainEventWriter", "SEND ObjectAddEvent RETURNING...."); 
        } catch ( ... ) {
          RH_NL_WARN("DomainEventWriter", "Unable to send the following ObjectAddedEvent (the event service might not be running):\n  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" source category: "<<evt.category);
        }
      }
      else {
	RH_NL_WARN("DomainEventWriter", "Unable to send the following ObjectAddedEvent (no publisher available):\n  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" source category: "<<evt.category);
      }

    };


  void DomainEventWriter::sendRemovedEvent( const char* producerId, 
					   const char* sourceId, 
					   const char* sourceName, 
					   StandardEvent::SourceCategoryType sourceCategory )
  {
    DomainStateEvent  evt;
    evt.prod_id = producerId;
    evt.source_id = sourceId;
    evt.source_name = sourceName;
    evt.category = sourceCategory;
    sendRemovedEvent(evt);
  }
			      

  void DomainEventWriter::sendRemovedEvent( const DomainStateEvent  &evt ) {
      if (pub) {
        StandardEvent::DomainManagementObjectRemovedEventType new_message;
        new_message.producerId = CORBA::string_dup(evt.prod_id.c_str());
        new_message.sourceId = CORBA::string_dup(evt.source_id.c_str());
        new_message.sourceName = CORBA::string_dup(evt.source_name.c_str());
        new_message.sourceCategory = evt.category;
        try {
          RH_NL_DEBUG("DomainEventWriter", "SEND ObjectRemovedEvent producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" source category: "<<evt.category);
          pub->push(new_message);
          RH_NL_DEBUG("DomainEventWriter", "SEND ObjectRemovedEvent RETURNING....");  
        } catch ( ... ) {
          RH_NL_WARN("DomainEventWriter", "Unable to send the following ObjectRemovedEvent (the event service might not be running):\n  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" source category: "<<evt.category);
        }
      }
      else {
	RH_NL_WARN("DomainEventWriter", "Unable to send the following ObjectRemovedEvent (no publisher available):\n  producer id:"<<evt.prod_id<<" source id:"<<evt.source_id<<" source name:"<<evt.source_name<<" source category: "<<evt.category);
      }
  };


  //
  // 
  //
  void SendStateChangeEvent(const char* producerId, 
			    const char* sourceId,
			    StandardEvent::StateChangeCategoryType stateChangeCategory, 
			    StandardEvent::StateChangeType stateChangeFrom, 
			    StandardEvent::StateChangeType stateChangeTo,
			    PublisherPtr  publisher )
  {
    DomainEventWriter ewriter(publisher);
    ewriter.sendObjectStateChange( producerId, 
				   sourceId, 
				   stateChangeCategory, 
				   stateChangeFrom, 
				   stateChangeTo );
  }


  void SendObjectRemovedEvent( const char* producerId, 
			       const char* sourceId, 
			       const char* sourceName, 
			       StandardEvent::SourceCategoryType sourceCategory, 
			       PublisherPtr publisher)
  {
    DomainEventWriter ewriter(publisher);
    ewriter.sendRemovedEvent( producerId, 
			      sourceId, 
			      sourceName,
			      sourceCategory );
  }


  void SendObjectAddEvent( const char* producerId, 
			   const char* sourceId, 
			   const char* sourceName, 
			   RH_Object src_obj,
			   StandardEvent::SourceCategoryType sourceCategory, 
			   PublisherPtr publisher)
  {
    DomainEventWriter ewriter(publisher);
    ewriter.sendAddedEvent( producerId, 
			    sourceId, 
			    sourceName, 
			    src_obj, 
			    sourceCategory);
  }


  void SendComponentTerminationEvent( const char* deviceId, 
                                      const char* applicationId, 
                                      const char* componentId,
                                      PublisherPtr publisher)
  {
    DomainEventWriter ewriter(publisher);
    ewriter.sendComponentTermination( deviceId,
                                      applicationId,
                                      componentId);
  }
  

  
  //
  // EM_Publisher
  //
  // Publisher interface supplied by a local Manager. Creates relationship to Manager
  // that allows for unregister operation.
  //
  // 
  class EM_Publisher : public Publisher {

  public:

    virtual ~EM_Publisher() {
      RH_NL_INFO("EM_Publisher", "DTOR START");
      // unregister the object with the Manager
      _ecm._unregister( _creg );
      RH_NL_INFO("EM_Publisher", "DTOR END");
    };


  private: 

    friend class Manager;

    EM_Publisher( Manager &ecm, const ossie::events::EventChannelReg &reg ) :
      Publisher( reg.channel ),
      _ecm( ecm ),
      _creg(reg)
    {};
    
    Manager               &_ecm;
    ossie::events::EventChannelReg       _creg;

  };


  //
  // EM_Subscriber
  //
  // Subscriber interface supplied by a local Manager. Creates relationship to Manager
  // that allows for unregister operation.
  //
  // 
  class EM_Subscriber : public Subscriber {

  public:

    virtual ~EM_Subscriber() {
      RH_NL_INFO("EM_Subscriber", "DTOR START");
      // unregister the object with the Manager
      _ecm._unregister( _creg );
      RH_NL_INFO("EM_Subscriber", "DTOR END");
    };

  private: 

    friend class Manager;

    EM_Subscriber( Manager &ecm,  const ossie::events::EventChannelReg &reg ) :
      Subscriber( reg.channel ),
      _ecm( ecm ),
      _creg(reg)
    {};
    
    Manager               &_ecm;
    ossie::events::EventChannelReg       _creg;

  };



  typedef boost::shared_ptr< EM_Publisher >   EM_PublisherPtr;
  typedef boost::shared_ptr< EM_Subscriber >  EM_SubscriberPtr;

  // Class singleton for this library
  ManagerPtr Manager::_Manager;


  ManagerPtr Manager::GetManager( Resource_impl *obj ) {

    std::string oid("");
    if ( obj ) {
      oid = ossie::corba::returnString(obj->identifier());
    }

    if ( !Manager::_Manager ) {
      try {
        Manager::_Manager = boost::shared_ptr<Manager>(new Manager( obj ));
        RH_NL_DEBUG("redhawk::events::Manager",  "Created EventManager for Resource: " << oid );
      }
      catch(...){
        RH_NL_ERROR("redhawk::events::Manager",  "Resource (" <<oid << ") does not provide EventChannelManager access, Event channel management is not allowed");
      }

    }
    return Manager::_Manager;
    
  }


  void  Manager::Terminate() {

    // release all Publishers and Subscribers
    RH_NL_INFO("redhawk::events::Manager",  "Terminate all EventChannels");
    if ( Manager::_Manager ) _Manager->_terminate();
  }


  Manager::Manager( Resource_impl *obj ) :
    _allow(true),
    _obj(NULL),
    _obj_id("")
  {

    if ( obj  ){
      _obj = obj;
      _obj_id = ossie::corba::returnString(obj->identifier());
      RH_NL_DEBUG("redhawk::events::Manager",  "Resolve Device and Domain Managers...");
      // setup create publisher as internal methods
      redhawk::DomainManagerContainer *dm = obj->getDomainManager();
      if ( dm == NULL ) throw -1;

      CF::DomainManager_ptr  domMgr =  dm->getRef();
      if ( !ossie::corba::objectExists( domMgr ) ) throw 1;
      RH_NL_DEBUG("redhawk::events::Manager",  "Resolved Domain Manager...");

      CF::EventChannelManager_ptr  ecm = domMgr->eventChannelMgr();
      RH_NL_DEBUG("redhawk::events::Manager",  "Getting Event Channel Manager...");
      if ( !ossie::corba::objectExists( ecm ) ) throw 1;
      _ecm = ecm;
    }

  };


  //
  // Clean up my resources with the Domain's EventChannelManager
  //
  Manager::~Manager() {

    // clean up any consumer and publisher we made
    if ( _registrations.size() > 0 ) {
      _terminate();
    }
      
  }

  //
  // Request a Publisher on a specified Event Channel
  //
  PublisherPtr Manager::Publisher( const std::string &channel_name, const std::string &registrationId ) 
    throw (RegistrationExists, RegistrationFailed) {
    SCOPED_LOCK(_mgr_lock);

    RH_NL_DEBUG("redhawk::events::Manager",  "Requesting Publisher for Channel:" << channel_name  << " resource:" << _obj_id );

    EM_PublisherPtr pub;

    try {
      
      if ( ossie::corba::objectExists(_ecm) ) {
	ossie::events::EventRegistration         ereg;
	ossie::events::EventChannelReg_var       registration;
	ossie::events::EventChannelReg           reg;
        ereg.channel_name = CORBA::string_dup(channel_name.c_str());
        ereg.reg_id = CORBA::string_dup(registrationId.c_str());
        
        RH_NL_DEBUG("redhawk::events::Manager",  "Requesting Channel:" << channel_name  << " from Domain's EventChannelManager " );
        registration = _ecm->registerResource( ereg );
        reg = registration.in();
        pub = EM_PublisherPtr( new EM_Publisher( *this, reg ) );
        
        RH_NL_INFO("redhawk::events::Manager",  "PUBLISHER - Channel:" << channel_name  << " Reg-Id" << registration->reg.reg_id << " RESOURCE:" << _obj_id  );
        _registrations.push_back( reg );

      }
    }
    catch( CF::EventChannelManager::RegistrationAlreadyExists e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Publisher for Channel:" << channel_name << ", REASON: Registration already exists.");
      throw RegistrationExists();
    }
    catch( CF::EventChannelManager::InvalidChannelName e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Publisher for Channel:" << channel_name << ", REASON: Invalid channel name.");
      throw RegistrationFailed();
    }
    catch( CF::EventChannelManager::OperationFailed e) {  
      RH_NL_ERROR("EventChannelManager", "Unable to create Publisher for Channel:" << channel_name << ", REASON: Operation failed.");
      throw RegistrationFailed();
    }
    catch( CF::EventChannelManager::OperationNotAllowed e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Publisher for Channel:" << channel_name << ", REASON: Operation failed.");
      throw RegistrationFailed();
    }
    catch( CF::EventChannelManager::ServiceUnavailable e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Publisher for Channel:" << channel_name << ", REASON: Service unavailable.");
      throw RegistrationFailed();
    }

    return pub;

  }

  SubscriberPtr  Manager::Subscriber( const std::string &channel_name, const std::string &registrationId )
    throw (RegistrationExists, RegistrationFailed) {

    SCOPED_LOCK(_mgr_lock);

    EM_SubscriberPtr sub;
    RH_NL_DEBUG("redhawk::events::Manager",  "Requesting Subscriber, for Channel:" << channel_name << " resource:" << _obj_id );

    try {
      
      if ( ossie::corba::objectExists(_ecm) ) {
        ossie::events::EventRegistration              ereg;
	ossie::events::EventChannelReg_var            registration;
	ossie::events::EventChannelReg                reg;
        ereg.channel_name = CORBA::string_dup(channel_name.c_str());
        ereg.reg_id = CORBA::string_dup(registrationId.c_str());
        
        RH_NL_DEBUG("redhawk::events::Manager",  "Requesting Channel:" << channel_name  << " from Domain's EventChannelManager " );
        registration = _ecm->registerResource( ereg );
	reg = registration.in();
        sub = EM_SubscriberPtr( new EM_Subscriber( *this, reg ) );
        
        RH_NL_INFO("redhawk::events::Manager",  "SUBSCRIBER - Channel:" << channel_name  << " Reg-Id" << registration->reg.reg_id  << " resource:" << _obj_id );
        _registrations.push_back( reg  );

      }
    }
    catch( CF::EventChannelManager::RegistrationAlreadyExists e ) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Subscriber for Channel:" << channel_name << ", REASON: Registration already exists.");
      throw RegistrationExists();
    }
    catch( CF::EventChannelManager::InvalidChannelName e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Subscriber for Channel:" << channel_name << ", REASON: Invalid channel name.");
      throw RegistrationFailed();
    }
    catch( CF::EventChannelManager::OperationFailed e) {  
      RH_NL_ERROR("EventChannelManager", "Unable to create Subscriber for Channel:" << channel_name << ", REASON: Operation failed.");
      throw RegistrationFailed();
    }
    catch( CF::EventChannelManager::OperationNotAllowed e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Subscriber for Channel:" << channel_name << ", REASON: Operation failed.");
      throw RegistrationFailed();
    }
    catch( CF::EventChannelManager::ServiceUnavailable e) { 
      RH_NL_ERROR("EventChannelManager", "Unable to create Subscriber for Channel:" << channel_name << ", REASON: Service unavailable.");
      throw RegistrationFailed();
    } 

    return sub;
  }


  void   Manager::_terminate() {

    SCOPED_LOCK(_mgr_lock);
    _allow = false;

    RH_NL_INFO("redhawk::events::Manager",  " Resource: " << _obj_id << ", Terminate All Registrations.: " << _registrations.size() );
    Registrations::iterator iter = _registrations.begin();
      for ( ; iter != _registrations.end();  iter++ ) {

        const ossie::events::EventChannelReg creg= (*iter);
            
	if ( ossie::corba::objectExists(_ecm) ) {
	  try {
	    // unregister from the Domain
            RH_NL_INFO("redhawk::events::Manager",  "Unregister REG=ID:" << creg.reg.reg_id );
	    _ecm->unregister( creg.reg );
	  }
	  catch(...) {
	    /// log error
            RH_NL_ERROR("redhawk::events::Manager",  "Unregister ERROR REG=ID:" << creg.reg.reg_id );
	  }
	}
      }

      // need to cleanup Publisher memory
      _registrations.clear();

    RH_NL_INFO("redhawk::events::Manager",  "Terminate Completed.");

  }


  void   Manager::_unregister( const ossie::events::EventChannelReg &reg ) {

    if (!_allow) return;

    SCOPED_LOCK(_mgr_lock);
    std::string regid( reg.reg.reg_id.in() );
    RH_NL_DEBUG("redhawk::events::Manager",  "Unregister request .... reg-id:" << regid );
    Registrations::iterator iter = _registrations.begin();
    for ( ; iter != _registrations.end();  iter++ ) {
        
      RH_NL_DEBUG("redhawk::events::Manager", "TBL REG-ID:" << iter->reg.reg_id << " REQ:" << reg.reg.channel_name );
      if ( regid.compare(iter->reg.reg_id) == 0 ){

          RH_NL_DEBUG("redhawk::events::Manager", "FOUND REG-ID:" << regid << " CHANNEL:" << reg.reg.channel_name );
          if ( ossie::corba::objectExists(_ecm) ) {
            try {
              // unregister from the Domain
              RH_NL_INFO("redhawk::events::Manager", "UNREGISTER REG-ID:" << regid << " CHANNEL:" << reg.reg.channel_name );
              _ecm->unregister( reg.reg );
            }
            catch(...) {
              /// log error
              RH_NL_ERROR("redhawk::events::Manager", "UNREGISTER FAILED, REG-ID:" << regid << " CHANNEL:" << reg.reg.channel_name );
            }
            
            _registrations.erase(iter);
            break;
          }
        }
    }
  }


  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  /**

     redhawk::events::Publisher

   */


  class DefaultSupplier : virtual public ossie::events::EventPublisherSupplierPOA {
    public:
      virtual ~DefaultSupplier() {};
      virtual void disconnect_push_supplier () 
    {
      RH_NL_DEBUG("DefaultSupplier", "handle disconnect_push_supplier." );
      parent.proxy = ossie::events::EventPublisher::_nil();
    } ;

  private:
    friend class Publisher;

    DefaultSupplier ( Publisher &parent ) : 
      parent(parent)
    {};

  protected:
    Publisher                                  &parent;
  };

  Publisher::Publisher( ossie::events::EventChannel_ptr   inChannel ) :
    channel(inChannel),
    supplier(NULL),
    is_local(true)
    {
      // if user passes a bad param then throw...
      if ( CORBA::is_nil(inChannel) == true ) throw (CF::EventChannelManager::OperationNotAllowed());

      // create a local supplier object 
      supplier = new DefaultSupplier(*this);

      connect();
    }

  //
  //  Publisher for an Event Channel
  //
  // @param channel    event channel to create a subscriber for
  // @apram 
  // @param supplier   actual supplier object that pushes information to event channel
  // @param retries    number of retries to perform when trying to establish  publisher interface
  // @param retry_wait number of millisecs to wait between retries
  Publisher::Publisher( ossie::events::EventChannel_ptr   inChannel,
                        ossie::events::EventPublisherSupplierPOA   *supplier ):
    channel(inChannel),
    supplier(supplier),
    is_local(false)
    {
      if ( CORBA::is_nil(inChannel) == true ) throw (CF::EventChannelManager::OperationNotAllowed());

      connect();
    }


  Publisher::~Publisher() {

    RH_NL_INFO("Publisher", "DTOR - START." );

    try {
      disconnect();
    }
    catch(...){
    }

    if ( supplier ) {
      supplier->_remove_ref();
    }
    proxy = ossie::events::EventPublisher::_nil();
    supplier=NULL;
    channel=ossie::events::EventChannel::_nil();

    RH_NL_DEBUG("Publisher", "DTOR - END." );

  }


  void Publisher::_init( ) {

  };


 int Publisher::connect(const int retries,  const int retry_wait )
 {
   int retval=-1;
   int tries = retries;

   if ( CORBA::is_nil(channel) == true ) return retval;

   if ( CORBA::is_nil(proxy) ) {
     //
     // Get Supplier Admin interface 
     //
     CosEventChannelAdmin::SupplierAdmin_var     supplier_admin;
     do
       {
         try {
           supplier_admin = channel->for_suppliers ();
           break;
         }
         catch (CORBA::COMM_FAILURE& ex) {
         }
         if ( retry_wait > 0 ) {
           boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
         } else {
           boost::this_thread::yield();
         }
         tries--;
       } while ( tries );

     if ( CORBA::is_nil(supplier_admin) ) return retval;
    
     RH_NL_DEBUG("Publisher", "Obtained SupplierAdmin." );

     tries=retries;
     do {
       try {
         proxy = supplier_admin->obtain_push_consumer ();
         break;
       }
       catch (CORBA::COMM_FAILURE& ex) {
       }
       if ( retry_wait > 0 ) {
         boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
       } else {
         boost::this_thread::yield();
       }
       tries--;
     } while ( tries );
   }

   RH_NL_DEBUG("Publisher", "::connect Checking for proxy." );
   if ( CORBA::is_nil(proxy) ) return retval;

   ossie::events::EventPublisherSupplier_var sptr =  ossie::events::EventPublisherSupplier::_nil();
   if ( supplier != NULL ) {
     try {
       RH_NL_DEBUG("Publisher", "::connect pushing Suppiler servant to POA ." );
        PortableServer::POA_ptr poa = supplier->_default_POA();
        if (!CORBA::is_nil(poa) )  {
          PortableServer::ObjectId_var oid = poa->servant_to_id(supplier);
          RH_NL_DEBUG("Publisher", "::connect activiting Suppiler servant." );
          poa->activate_object_with_id( oid, supplier );
        }
     }
     catch(...) {
     }
     sptr = supplier->_this();
        
   }

   // now attach supplier to the proxy
   tries = retries;
   do {
     try {
       proxy->connect_push_supplier(sptr.in());
       retval=0;
       RH_NL_DEBUG("Publisher", "::connect, Connect Supplier to EventChannel....." );
       break;
     }
     catch (CORBA::BAD_PARAM& ex) {
       RH_NL_ERROR("Publisher", "::connect, Caught BAD_PARAM to connect_push_supplier operation." );
       break;
     }
     catch (CosEventChannelAdmin::AlreadyConnected& ex) {
       retval=0;
       break;
     }
     catch (CORBA::COMM_FAILURE& ex) {
       RH_NL_ERROR("Publisher",  "::connect Caught COMM_FAILURE Exception "  << 
                   "connecting Push Supplier! Retrying..." );
     }
     if ( retry_wait > 0 ) {
       boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
     } else {
       boost::this_thread::yield();
     }
     tries--;
   } while ( tries );

   return retval;
 }


int Publisher::disconnect(const int retries,  const int retry_wait ) 
{
  int retval=-1;

  RH_NL_DEBUG("Publisher", "::disconnect deactivate supplier." );
  if ( supplier ){
    try {
      PortableServer::POA_ptr poa = supplier->_default_POA();
      PortableServer::ObjectId_var oid = poa->servant_to_id(supplier);
      poa->deactivate_object( oid );
    }
    catch (...) {
    }
  }

  RH_NL_DEBUG("Publisher", "::disconnect Checking for proxy." );
  if ( CORBA::is_nil(proxy) ) return retval;

  int tries = retries;
  do {
    try {
      proxy->disconnect_push_consumer();
      retval=0;
      break;
    }
    catch (CORBA::COMM_FAILURE& ex) {
      RH_NL_ERROR("Publisher",  "::disconnect, Caught COMM_FAILURE Exception "
                  << "disconnecting Push Supplier! Retrying..." );
    }
    catch (...) {
      RH_NL_ERROR("Publisher",  "::disconnect, UNKOWN Exception "
                  << "disconnecting Push Consumer! Retrying..." );
    }
    if ( retry_wait > 0 ) {
      boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
    } else {
      boost::this_thread::yield();
    }
    tries--;
  } while(tries);
  RH_NL_DEBUG("Publisher",  "::disconnect ProxyPushConsumer disconnected." );     

  return retval;

}



  int     Publisher::push( const std::string &msg ) {
      int retval=0;
      try {
        CORBA::Any data;
        data <<= msg.c_str();
        if (!CORBA::is_nil(proxy)) {
          proxy->push(data);
        }
        else{
          retval=-1;
        }
      }
      catch( CORBA::Exception& ex) {
        retval=-1;
      }
      return retval;
    }


  int     Publisher::push( CORBA::Any &data ) {
      int retval=0;
      try {
        if (!CORBA::is_nil(proxy)) {
          proxy->push(data);
        }
        else{
          retval=-1;
        }
      }
      catch( CORBA::Exception& ex) {
        retval=-1;
      }
      return retval;

    }



  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  /**

     redhawk::events::Supplier

   */

  //
  // used for boost shared pointer instantion when user
  // supplied callback is provided
  //
  struct null_deleter  {
    void operator()(void const *) const
    {
    }
  };

  /**
   * Wrap Callback functions as ConnectionEventListener objects
   */
  class StaticDataArrivedListener : public Subscriber::DataArrivedListener  {
  public:
    virtual void operator() ( const CORBA::Any &data )
    {
      (*func_)(data);
    }

    StaticDataArrivedListener ( Subscriber::DataArrivedCallbackFn func) :
      func_(func)
    {
    }

  private:

    Subscriber::DataArrivedCallbackFn func_;
  };

  //
  // CallbackConsumer provided to Subscriber
  //
  class CallbackConsumer : public ossie::events::EventSubscriberConsumerPOA {
  public:
    virtual ~CallbackConsumer() {};
    virtual void disconnect_push_consumer() {};
    virtual void push( const CORBA::Any &data ) {
      if ( callback ) {
        try{
          (*callback)( data );
        }
        catch(...){
        }
      };
    };

  private:
    friend class Subscriber;

    CallbackConsumer ( Subscriber &parent, Subscriber::DataArrivedListenerPtr  callback ) : 
      parent(parent),
      callback(callback)
    {
    } ;

  protected:
    Subscriber                                  &parent;
    Subscriber::DataArrivedListenerPtr          callback;

  };


  class DefaultConsumer : public ossie::events::EventSubscriberConsumerPOA {
  public:
    virtual ~DefaultConsumer() {};
    virtual void disconnect_push_consumer() {
      RH_NL_DEBUG("DefaultConsumer", "handle disconnect_push_consumer." );
      if ( CORBA::is_nil(parent.proxy) == false ){
	parent.proxy = ossie::events::EventSubscriber::_nil();
      }
    };
    virtual void push( const CORBA::Any &data ) {
      // if parent defines a callback
      if ( parent.dataArrivedCB ) {
        try{
          RH_NL_DEBUG("Subscriber", "DefaultConsumer --> send to callback." );
            (*parent.dataArrivedCB)( data );
          }
          catch(...){
          }
      }
      else {
        RH_NL_DEBUG("Subscriber", "DefaultConsumer --> push onto event queue." );
        parent.events.push_back(data);
      }
    };

  private:
    friend class Subscriber;

    DefaultConsumer ( Subscriber &parent ) : 
      parent(parent)
    {};

  protected:
    Subscriber                                  &parent;
  };



  Subscriber::Subscriber(  ossie::events::EventChannel_ptr  inChannel ):
    channel(inChannel),
    consumer(NULL)
  {
    if ( CORBA::is_nil(inChannel) == true ) throw (CF::EventChannelManager::OperationNotAllowed());
    _init();
  }


  Subscriber::Subscriber(  ossie::events::EventChannel_ptr  inChannel,
                           DataArrivedListener *newListener ):
    channel(inChannel),
    consumer(NULL)
  {
    dataArrivedCB =  boost::shared_ptr< DataArrivedListener >(newListener, null_deleter());
    if ( CORBA::is_nil(inChannel) == true ) throw (CF::EventChannelManager::OperationNotAllowed());
    _init();
  }


  Subscriber::Subscriber(  ossie::events::EventChannel_ptr  inChannel,
                           DataArrivedCallbackFn  newListener ) :
    channel(inChannel),
    consumer(NULL)
  {
    dataArrivedCB =  boost::make_shared< StaticDataArrivedListener >( newListener );
    if ( CORBA::is_nil(inChannel) == true ) throw (CF::EventChannelManager::OperationNotAllowed());
    _init();
  }



  Subscriber::~Subscriber( ) {

    RH_NL_TRACE("Subscriber", "DTOR - START." );

    try {
      disconnect();
    }
    catch(...){
    }

    if ( consumer ) {
      consumer->_remove_ref();
    }

    proxy = ossie::events::EventSubscriber::_nil();
    consumer = NULL;
    channel=ossie::events::EventChannel::_nil();

    RH_NL_DEBUG("Subscriber", "DTOR - END." );
  }


  void Subscriber::_init( ) 
  {
    if  ( consumer == NULL ) {
      RH_NL_DEBUG("Subscriber", "Create local DefaultConsumer for EventChannel Subscriber." );
      consumer = new DefaultConsumer(*this );
      if ( consumer == NULL ) return;
    }

    connect();
  };


int Subscriber::disconnect(const int retries,  const int retry_wait )
{  
  int retval=-1;
  int tries = retries;

  if ( consumer ){
    try {
      PortableServer::POA_ptr poa = consumer->_default_POA();
      PortableServer::ObjectId_var oid = poa->servant_to_id(consumer);
      poa->deactivate_object( oid );
    }
    catch (...) {
    }
  }


  if ( CORBA::is_nil(proxy) == false ) {
    do {
      try {
        RH_NL_DEBUG("Subscriber",  "disconnect_push_supplier... ");
        proxy->disconnect_push_supplier();
        retval=0;
        break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
        RH_NL_WARN("Subscriber",  "Caught COMM_FAILURE Exception "
                   << "disconnecting Push Supplier! Retrying..." );
      }
      catch (...) {
        RH_NL_ERROR("Publisher",  "::disconnect, UNKOWN Exception "
                    << "disconnecting Push Supplier! Retrying..." );
      }
      if ( retry_wait > 0 ) {
        boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
        boost::this_thread::yield();
      }
      tries--;
    } while(tries);
    RH_NL_TRACE("Subscriber", "ProxyPushSupplier disconnected." );
  }

  return retval;

}


int Subscriber::connect(const int retries,  const int retry_wait ) {

  int retval=-1;
  int tries=retries;

  if ( CORBA::is_nil(channel) ) {
    return -1;
  }

  CosEventChannelAdmin::ConsumerAdmin_var     consumer_admin;
  if ( CORBA::is_nil(proxy) ) {
    do
      {
	try {
	  consumer_admin = channel->for_consumers ();
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    if ( CORBA::is_nil(consumer_admin) ) return retval;
    
    RH_NL_DEBUG("Subscriber", "Obtained ConsumerAdmin." );

    tries=retries;
    do {
      try {
        proxy = consumer_admin->obtain_push_supplier ();
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );
  }

  RH_NL_DEBUG("Subscriber", "Obtained ProxyPushConsumer." );
  if ( CORBA::is_nil(proxy) ) return retval;

  CosEventComm::PushConsumer_var sptr = CosEventComm::PushConsumer::_nil();
  if  ( consumer != NULL ) {
     try {
        PortableServer::POA_ptr poa = consumer->_default_POA();
        if (!CORBA::is_nil(poa) )  {
          PortableServer::ObjectId_var oid = poa->servant_to_id(consumer);
          poa->activate_object_with_id( oid, consumer );
        }
     }
     catch(...) {
     }
     sptr = consumer->_this();
  }
  else {
      RH_NL_DEBUG("Subscriber", "Create local DefaultConsumer for EventChannel Subscriber." );
      consumer = new DefaultConsumer(*this );
      if ( consumer == NULL ) return retval;
    sptr = consumer->_this();
  }

  // now attach supplier to the proxy
  tries=retries;
  do {
    try {
      // connect the the consumer object to the supplier's proxy
      proxy->connect_push_consumer( sptr.in() );
      retval=0;
      RH_NL_DEBUG("Subscriber", "Consumer is now attached to event channel, (connect method)" );
      break;
    }
    catch (CORBA::BAD_PARAM& ex) {
      RH_NL_ERROR("Subscriber", "Caught BAD_PARAM for connect_push_consumer operation." );
      break;
    }
    catch (CosEventChannelAdmin::AlreadyConnected& ex) {
      retval=0;
      break;
    }
    catch (CORBA::COMM_FAILURE& ex) {
      RH_NL_ERROR("Subscriber",  "Caught COMM_FAILURE Exception "  << 
                  "connecting Push Consumer! Retrying..." );
    }
    if ( retry_wait > 0 ) {
      boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
    } else {
      boost::this_thread::yield();
    }
    tries--;
  } while ( tries );

  RH_NL_DEBUG("Subscriber",  "Connected Push Consumer." );
  return retval;
}



  void   Subscriber::setDataArrivedListener( DataArrivedListener *newListener ) {
    dataArrivedCB =  boost::shared_ptr< DataArrivedListener >(newListener, null_deleter());
  }

  void   Subscriber::setDataArrivedListener( DataArrivedCallbackFn  newListener ) {
    dataArrivedCB =  boost::make_shared< StaticDataArrivedListener >( newListener );
  }


int Subscriber::getData( std::string &ret_msg ) {
      RH_NL_DEBUG("Subscriber",  " getdata (std::string) msg :" << ret_msg);        
      int retval=-1;
      try{

        // check if callback method is enable.. it so then return
        RH_NL_DEBUG("Subscriber",  " test for callback ");                  
        if ( dataArrivedCB ) return retval;

        // check if data is available
        RH_NL_DEBUG("Subscriber",  " queue size "<< events.size());                  
        if ( events.size() < 1 ) return retval;

        RH_NL_DEBUG("Subscriber",  " getdata (std::string) front ");                  
        CORBA::Any  rawdata =  events.front();
        RH_NL_DEBUG("Subscriber",  " getdata (std::string) after front:");        
          
        const char *tmsg;
        RH_NL_DEBUG("Subscriber",  " getdata (std::string) calling CORBA extract operator >>=");        
        if (rawdata >>= tmsg) { 
          ret_msg = tmsg;
          RH_NL_DEBUG("Subscriber",  " extracted msg :" << ret_msg);
          retval=0;
        }

        events.pop_front();

      }
      catch(...) {
        RH_NL_ERROR("Subscriber",  "(getData::string) Error grabbing data from queue");
      }

      return retval;
    }

int Subscriber::getData( CORBA::Any &ret ) {
        
      int retval=-1;
      try{

        // check if callback method is enable.. it so then return
        if ( dataArrivedCB ) return retval;

        // check if data is available
        if ( events.size() < 1 ) return retval;
          
        CORBA::Any  rawdata =  events.front();
          
        ret = rawdata;
        retval=0;
        events.pop_front();

      }
      catch(...) {
        RH_NL_ERROR("Subscriber",  "(getData::Any) Error grabbing data from queue");
      }

      return retval;
    }



};  // end of events namespace

REDHAWK_CPP_NAMESPACE_END


