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


#ifndef NDN_ENCODING_ENCODER_TEST_HPP
#define NDN_ENCODING_ENCODER_TEST_HPP

#include "common.hpp"
#include "wire_test.hpp"

namespace ndn {
namespace encoding {

/**
 * @brief Helper class to perform TLV encoding
 * Interface of this class (mostly) matches interface of Estimator class
 * @sa Estimator
 */
class Encoder
{
public:   // common interface between Encoder and Estimator
  /**
   * @brief Create instance of the encoder with the specified reserved sizes
   * @param firstReserve initial the first buffer size to reserve
   */
  Encoder(size_t firstReserve);

  /**
   * @brief Create instance of the encoder from an existing @p wire
   */
  Encodr(const Wire& wire);

  /**
   * @brief Append a byte
   */
  size_t
  appendByte(uint8_t value);

  /**
   * @brief Append a byte array @p array of length @p length
   */
  size_t
  appendByteArray(const uint8_t* array, size_t length);
  
  /**
   * @brief Prepend VarNumber @p varNumber of NDN TLV encoding
   * @sa http://named-data.net/doc/ndn-tlv/
   */
  size_t
  appendVarNumber(uint64_t varNumber);

  /**
   * @brief Append non-negative integer @p integer of NDN TLV encoding
   * @sa http://named-data.net/doc/ndn-tlv/
   */
  size_t
  appendNonNegativeInteger(uint64_t integer);

  /**
   * @brief Append TLV block of type @p type and value from buffer @p array of size @p arraySize
   */
  size_t
  appendByteArrayBlock(uint32_t type, const uint8_t* array, size_t arraySize);

  /**
   * @brief Append TLV block @p block
   */
  size_t
  appendBlock(const BlockN& block);
    
private:
  Wire m_wire;

};

}
}

