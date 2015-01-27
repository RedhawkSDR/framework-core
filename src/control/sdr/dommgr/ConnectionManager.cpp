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

#include <ossie/CorbaIterator.h>

#include "ConnectionManager.h"
#include "DomainManager_impl.h"
#include "Endpoints.h"

typedef ossie::corba::Iterator<CF::ConnectionManager::ConnectionStatusType,
                               CF::ConnectionManager::ConnectionStatusType_out,
                               CF::ConnectionManager::ConnectionStatusSequence,
                               CF::ConnectionManager::ConnectionStatusSequence_out,
                               CF::ConnectionStatusIterator,
                               POA_CF::ConnectionStatusIterator> ConnectionStatusIter;

static CF::ConnectionManager::ConnectionStatusSequence* connectionListToSequence(const ossie::ConnectionList& connections)
{
    CF::ConnectionManager::ConnectionStatusSequence_var result = new CF::ConnectionManager::ConnectionStatusSequence();

    for (ossie::ConnectionList::const_iterator ii = connections.begin(); ii != connections.end(); ++ii) {
        CF::ConnectionManager::ConnectionStatusType status;
        status.usesEndpoint = ii->uses->toEndpointStatusType();
        status.providesEndpoint = ii->provides->toEndpointStatusType();
        status.connectionId = ii->identifier.c_str();
        status.connected = ii->connected;
        ossie::corba::push_back(result, status);
    }

    return result._retn();
}

ConnectionManager_impl::ConnectionManager_impl(DomainManager_impl* domainManager) :
    _domainManager(domainManager)
{
}

ConnectionManager_impl::~ConnectionManager_impl()
{
}

ossie::Endpoint* ConnectionManager_impl::requestToEndpoint(const CF::ConnectionManager::EndpointRequest& request)
{
    ossie::Endpoint* endpoint = 0;
    switch (request.endpoint._d()) {
    case CF::ConnectionManager::ENDPOINT_APPLICATION:
        endpoint = new ossie::ApplicationEndpoint(request.endpoint.waveformId());
        break;
    case CF::ConnectionManager::ENDPOINT_DEVICE:
        endpoint = new ossie::ComponentEndpoint(request.endpoint.deviceId());
        break;
    case CF::ConnectionManager::ENDPOINT_SERVICE:
        endpoint = new ossie::ServiceEndpoint(request.endpoint.serviceName());
        break;
    case CF::ConnectionManager::ENDPOINT_EVENTCHANNEL:
        endpoint = new ossie::EventChannelEndpoint(request.endpoint.channelName());
        break;
    }

    const std::string portName(request.portName);
    if (portName.empty()) {
        return endpoint;
    } else {
        return new ossie::PortEndpoint(endpoint, portName);
    }
}

void ConnectionManager_impl::connect(const CF::ConnectionManager::EndpointRequest& usesEndpoint, const CF::ConnectionManager::EndpointRequest& providesEndpoint, char*& connectionId)
{
    // If no connection ID was given, generate one
    if (strlen(connectionId) == 0) {
        CORBA::string_free(connectionId);
        connectionId = CORBA::string_dup(ossie::generateUUID().c_str());
    }

    std::auto_ptr<ossie::Endpoint> uses(requestToEndpoint(usesEndpoint));
    std::auto_ptr<ossie::Endpoint> provides(requestToEndpoint(providesEndpoint));
    ossie::ConnectionNode connection(uses.release(), provides.release(), connectionId);
    connection.connect(_domainManager->_connectionManager);
    _domainManager->_connectionManager.restoreConnection("", connection);
}

void ConnectionManager_impl::disconnect(const char* connectionId)
{
    _domainManager->_connectionManager.breakConnection("", connectionId);
}

CF::ConnectionManager::ConnectionStatusSequence* ConnectionManager_impl::connections()
{
    CF::ConnectionManager::ConnectionStatusSequence_var result = new CF::ConnectionManager::ConnectionStatusSequence();

    const ossie::ConnectionTable connection_table = _domainManager->_connectionManager.getConnections();
    for (ossie::ConnectionTable::const_iterator ii = connection_table.begin(); ii != connection_table.end(); ++ii) {
        CF::ConnectionManager::ConnectionStatusSequence_var items = connectionListToSequence(ii->second);
        ossie::corba::extend(result, items);
    }
    
    return result._retn();
}

void ConnectionManager_impl::listConnections(CORBA::ULong count, CF::ConnectionManager::ConnectionStatusSequence_out conns, CF::ConnectionStatusIterator_out iter)
{
    iter = ConnectionStatusIter::list(count, conns, connections());
}
