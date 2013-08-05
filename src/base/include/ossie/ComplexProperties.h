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

#ifndef COMPLEXPROPERTIES_H
#define COMPLEXPROPERTIES_H

#include "ossie/PropertyInterface.h"
#include "CF/cf.h"

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<float>& complexOutput) {
    CF::complexFloat* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<float>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<float>& complexInput) {
    CF::complexFloat tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<bool>& complexOutput) {
    CF::complexBoolean* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<bool>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<bool>& complexInput) {
    CF::complexBoolean tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<CORBA::ULong>& complexOutput) {
    CF::complexULong* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<CORBA::ULong>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<CORBA::ULong>& complexInput) {
    CF::complexULong tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<short>& complexOutput) {
    CF::complexShort* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<short>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<short>& complexInput) {
    CF::complexShort tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<unsigned char>& complexOutput) {
    CF::complexOctet* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<unsigned char>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<unsigned char>& complexInput) {
    CF::complexOctet tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<char >& complexOutput) {
    CF::complexChar* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<char>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<char>& complexInput) {
    CF::complexChar tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<unsigned short>& complexOutput) {
    CF::complexUShort* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<unsigned short>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<unsigned short>& complexInput) {
    CF::complexUShort tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<double>& complexOutput) {
    CF::complexDouble* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<double>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<double>& complexInput) {
    CF::complexDouble tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<CORBA::Long>& complexOutput) {
    CF::complexLong* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<CORBA::Long>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<CORBA::Long>& complexInput) {
    CF::complexLong tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<CORBA::LongLong>& complexOutput) {
    CF::complexLongLong* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<CORBA::LongLong>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<CORBA::LongLong>& complexInput) {
    CF::complexLongLong tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

/// Set the complex value from the any value
inline bool operator>>= (const CORBA::Any& inputAny, std::complex<CORBA::ULongLong>& complexOutput) {
    CF::complexULongLong* temp;
    if (!(inputAny >>= temp)) return false;
    complexOutput = std::complex<CORBA::ULongLong>(temp->real, temp->imag);
    return true;
}

/// Set the any value from the complex value
inline void operator<<= (CORBA::Any& outputAny, const std::complex<CORBA::ULongLong>& complexInput) {
    CF::complexULongLong tmpCfComplex;
    tmpCfComplex.real = complexInput.real();
    tmpCfComplex.imag = complexInput.imag();
    outputAny <<= tmpCfComplex;
}

#endif // PROPERTYINTERFACE_H
