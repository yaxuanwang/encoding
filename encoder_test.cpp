/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 */

#include "encoder_test.hpp"
#include "endian.hpp"

namespace ndn {
namespace encoding {


Encoder::Encoder(size_t firstReserve)
  : m_wire(new Wire(firstReserve))
{
}

Encoder::Encodr(const Wire& wire)
  : m_wire(wire)
{
}

size_t
Encoder::appendByte(uint8_t value)
{
  m_wire.reserve(1);
  m_wire.writeUint8(value);
  return 1;
}

size_t
Encoder::appendByteArray(const uint8_t* array, size_t length)
{
  m_wire.reserve(length);
  m_wire.appendArray(array,length);
  return length;
}

size_t
Encoder::appendVarNumber(uint64_t varNumber)
{
  if (varNumber < 253) {
    appendByte(static_cast<uint8_t>(varNumber));
    return 1;
  }
  else if (varNumber <= std::numeric_limits<uint16_t>::max()) {
    appendByte(253);
    uint16_t value = htobe16(static_cast<uint16_t>(varNumber));
    appendByteArray(reinterpret_cast<const uint8_t*>(&value), 2);
    return 3;
  }
  else if (varNumber <= std::numeric_limits<uint32_t>::max()) {
    appendByte(254);
    uint32_t value = htobe32(static_cast<uint32_t>(varNumber));
    appendByteArray(reinterpret_cast<const uint8_t*>(&value), 4);
    return 5;
  }
  else {
    appendByte(255);
    uint64_t value = htobe64(varNumber);
    appendByteArray(reinterpret_cast<const uint8_t*>(&value), 8);
    return 9;
  }
}

size_t
Encoder::appendNonNegativeInteger(uint64_t varNumber)
{
  if (varNumber <= std::numeric_limits<uint8_t>::max()) {
    return appendByte(static_cast<uint8_t>(varNumber));
  }
  else if (varNumber <= std::numeric_limits<uint16_t>::max()) {
    uint16_t value = htobe16(static_cast<uint16_t>(varNumber));
    return appendByteArray(reinterpret_cast<const uint8_t*>(&value), 2);
  }
  else if (varNumber <= std::numeric_limits<uint32_t>::max()) {
    uint32_t value = htobe32(static_cast<uint32_t>(varNumber));
    return appendByteArray(reinterpret_cast<const uint8_t*>(&value), 4);
  }
  else {
    uint64_t value = htobe64(varNumber);
    return appendByteArray(reinterpret_cast<const uint8_t*>(&value), 8);
  }
}

size_t
Encoder::appendByteArrayBlock(uint32_t type, const uint8_t* array, size_t arraySize)
{
  size_t totalLength = appendVarNumber(type);
  totalLength += appendVarNumber(arraySize);
  totalLength += appendByteArray(array, arraySize);

  return totalLength;
}

size_t
Encoder::appendBlock(const BlockN& block)
{
  size_t length = m_wire.appendBlock(block);
  return length;
}

}
}
