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
#include "ossie/Component.h"

Component::Component(const char* _uuid) : Resource_impl (_uuid) {
    this->_app = NULL;
}

Component::Component(const char* _uuid, const char *label) : Resource_impl (_uuid, label) {
    this->_app = NULL;
}

Component::~Component() {
    if (this->_app != NULL)
        delete this->_app;
}

void Component::setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior)
{
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    Resource_impl::setAdditionalParameters(softwareProfile, application_registrar_ior);
    CORBA::Object_var applicationRegistrarObject = orb->string_to_object(application_registrar_ior.c_str());
    CF::ApplicationRegistrar_ptr applicationRegistrar = ossie::corba::_narrowSafe<CF::ApplicationRegistrar>(applicationRegistrarObject);
    if (!CORBA::is_nil(applicationRegistrar)) {
        this->_app = new redhawk::ApplicationContainer(applicationRegistrar->app());
        return;
    }
    this->_app = new redhawk::ApplicationContainer();
}
