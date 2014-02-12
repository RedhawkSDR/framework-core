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

#include <iostream>

#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/internal/orbParameters.h>

#include "ossie/CorbaUtils.h"

static CORBA::ORB_var orb = CORBA::ORB::_nil();
static PortableServer::POA_var root_poa = PortableServer::POA::_nil();
static CosNaming::NamingContext_var inc = CosNaming::NamingContext::_nil();

static bool persistenceEnabled = false;

namespace ossie {
namespace corba {

CREATE_LOGGER(CorbaUtils);

CORBA::ORB_ptr CorbaInit (int argc, char* argv[])
{
    // Initialize the ORB.
    CORBA::ORB_ptr orb = OrbInit(argc, argv, false);

    // Automatically activate the root POA manager.
    PortableServer::POAManager_var manager = RootPOA()->the_POAManager();
    manager->activate();

    return orb;
}

CORBA::ORB_ptr OrbInit (int argc, char* argv[], bool persistentIORs)
{
    if (CORBA::is_nil(orb)) {
        if (persistentIORs) {
            persistenceEnabled = true;

            // Check for a user-specified ORB endpoint; if one is given,
            // do not override it. This allows multiple persistent endpoints
            // on the same machine.
            for (int ii = 1; ii < argc; ++ii) {
                if (strcmp(argv[ii], "-ORBendPoint") == 0) {
                    orb = CORBA::ORB_init(argc, argv);
                    return orb;
                }
            }
            
            const char* corba_args[][2] = {
                { "endPoint", "giop:tcp::5678" },
                { 0, 0 }
            };
 
            orb = CORBA::ORB_init(argc, argv, "omniORB4", corba_args);
       } else {
            orb = CORBA::ORB_init(argc, argv);
       }
    }

    return orb;
}

void OrbShutdown (bool wait)
{
    if (CORBA::is_nil(orb)) {
        return;
    }

    // Release our references to the initial naming context and root POA.
    inc = CosNaming::NamingContext::_nil();
    root_poa = PortableServer::POA::_nil();

    if (wait) {
        orb->shutdown(1);
        orb->destroy();
    } else {
        orb->shutdown(0);
    }

    // Release our reference to the ORB, which should cause it to be destroyed.
    orb = CORBA::ORB::_nil();
}

CORBA::ORB_ptr Orb ()
{
    return orb;
}

PortableServer::POA_ptr RootPOA ()
{
    if (CORBA::is_nil(root_poa) && !CORBA::is_nil(orb)) {
        CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
        root_poa = PortableServer::POA::_narrow(obj);
    }

    return root_poa;
}

unsigned long giopMaxMsgSize ()
{
    return omni::orbParameters::giopMaxMsgSize;
}

CosNaming::NamingContext_ptr InitialNamingContext ()
{
    if (CORBA::is_nil(inc) && !CORBA::is_nil(orb)) {
        CORBA::Object_var obj = orb->resolve_initial_references("NameService");
        inc = CosNaming::NamingContext::_narrow(obj);
    }

    return inc;
}


bool isPersistenceEnabled ()
{
    return persistenceEnabled;
}

bool isValidType (const CORBA::Any& lhs, const CORBA::Any& rhs)
{
    CORBA::TypeCode_var tc1 = lhs.type();
    CORBA::TypeCode_var tc2 = rhs.type();

    return (tc1->equal(tc2));
}

  //
  // convenience routine to convert stringified name to CosNaming path
  //
  CosNaming::Name str2name(const char* namestr)
  {
    CosNaming::Name name;
    CORBA::ULong nameLen=0;
    name.length(nameLen);

    std::string n =namestr;
    std::string::size_type pos=0;
    char last='/';
    while(true)
      {
	pos=n.find_first_not_of("/.",pos);
	if(std::string::npos==pos) break;
	std::string::size_type sep =n.find_first_of("/.",pos);
	std::string piece =n.substr(pos, (std::string::npos==sep? sep: sep-pos) );
	if(last=='/')
	  {
	    name.length(++nameLen);
	    name[nameLen-1].id=CORBA::string_dup(piece.c_str());
	  }
	else
	  {
	    name[nameLen-1].kind=CORBA::string_dup(piece.c_str());
	  }
	if(std::string::npos==sep) break;
	pos=sep;
	last=n[sep];
      }
    return name;
  }



CosNaming::Name* stringToName (const std::string& name)
{
    return omni::omniURI::stringToName(name.c_str());
}

CORBA::Object_ptr objectFromName (const std::string& name)
{
    CosNaming::Name_var cosName = stringToName(name);
    return InitialNamingContext()->resolve(cosName);
}

std::string objectToString (const CORBA::Object_ptr obj)
{
    return returnString(orb->object_to_string(obj));
}

CORBA::Object_ptr stringToObject (const std::string& ior)
{
    return orb->string_to_object(ior.c_str());
}

 std::vector<std::string> listRootContext( ) {
    std::vector<std::string> t;
    if ( CORBA::is_nil(inc) == false )
      return listContext( InitialNamingContext(),"" );
    else
      return t;
  }

  std::vector<std::string> listContext(const CosNaming::NamingContext_ptr ctx, const std::string &dname ) {
    CosNaming::BindingIterator_var bi;
    CosNaming::BindingList_var bl;
    CosNaming::Binding_var b;
    const CORBA::ULong CHUNK = 0;
    
    //std::cout << " DIR:" << dname << std::endl;
    std::vector<std::string> t;
    try{
      ctx->list(CHUNK, bl, bi);
      while ( CORBA::is_nil(bi) == false &&  bi->next_one(b) ) {
	CORBA::String_var s = CORBA::string_dup(b->binding_name[0].id);
	std::string bname = s.in();
	if ( b->binding_type == CosNaming::nobject ) {
	  std::string n = dname;
	  n = n + "/" + bname;
	  //std::cout << " OBJ:" << n << std::endl;
	  t.push_back( n );
	}
	else if ( b->binding_type == CosNaming::ncontext ) {
	  std::vector< std::string > slist;
	  CORBA::Object_ptr obj=ctx->resolve( b->binding_name );
	  if ( CORBA::is_nil(obj) == false ) {
	    CosNaming::NamingContext_ptr nc= CosNaming::NamingContext::_narrow(obj);
	    std::string sdir=dname;
	    sdir = sdir+"/"+ bname;
	    slist = listContext( nc, sdir );
	    t.insert(t.end(), slist.begin(), slist.end() );
	  }
	}
      }
	
    }
    catch(...) {
      // skip to end
    }

    return t;
  }




void bindObjectToName (const CORBA::Object_ptr obj, const std::string& name)
{
    bindObjectToContext(obj, InitialNamingContext(), name);
}

void bindObjectToContext(const CORBA::Object_ptr obj, const CosNaming::NamingContext_ptr context, const std::string& name)
{
    CosNaming::Name_var cosName = stringToName(name);
    context->rebind(cosName, obj);
}

unsigned int numberBoundObjectsToContext(CosNaming::NamingContext_ptr context)
{
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    context->list(CHUNK, bl, it);

    return bl->length();
}



void unbindAllFromContext(CosNaming::NamingContext_ptr context)
{
    ///\todo Add support for deleting more than 100 names
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    context->list(CHUNK, bl, it);

    for (unsigned int ii = 0; ii < bl->length(); ++ii) {
        context->unbind(bl[ii].binding_name);
    }
}


PortableServer::ObjectId* activatePersistentObject (PortableServer::POA_ptr poa, PortableServer::Servant servant, const std::string& identifier)
{
    if (!persistenceEnabled) {
        return poa->activate_object(servant);
    }

    PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(identifier.c_str());
    poa->activate_object_with_id(oid, servant);
    return oid._retn();
}


static CORBA::Boolean handleCommFailure (void* cookie, CORBA::ULong retry, const CORBA::COMM_FAILURE& ex)
{
    int maxRetries = reinterpret_cast<long>(cookie);
    if (maxRetries < 0) {
        if (retry == 0) {
            LOG_WARN(CorbaUtils, "CORBA::COMM_FAILURE retrying indefinitely");
        }
        return true;
    } else if ((int)retry < maxRetries) {
        LOG_WARN(CorbaUtils, "CORBA::COMM_FAILURE retrying " << retry);
        return true;
    } else {
        LOG_WARN(CorbaUtils, "CORBA::COMM_FAILURE not retrying " << retry);
        return false;
    }
}


void setCommFailureRetries (int numRetries)
{
    omniORB::installCommFailureExceptionHandler(reinterpret_cast<void*>(numRetries), handleCommFailure);
}


void setObjectCommFailureRetries (CORBA::Object_ptr obj, int numRetries)
{
    omniORB::installCommFailureExceptionHandler(obj, reinterpret_cast<void*>(numRetries), handleCommFailure);
}

}; // namespace corba
}; // namespace ossie
