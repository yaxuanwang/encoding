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

#ifndef NDN_ENCODING_BLOCK_TEST_HPP 
#define NDN_ENCODING_BLOCK_TEST_HPP
 
#include "../common.hpp"
 
#include "buffer.hpp"
#include "tlv_test.hpp"
#include "encoding-buffer-fwd.hpp"

namespace boost {
namespace asio {
class const_buffer;
} // namespace asio
} // namespace boost

namespace ndn {

/** @brief Class representing a single element to construct a buffer wire of TLV format
 */
class BlockN
{
public:	
  class Error : public tlv::Error
    {
    public:
      explicit
      Error(const std::string& what)
        : tlv::Error(what)
      {
      }
    };
  
public: // constructor
  /** @brief Create an empty Block
   */
  BlockN();

  /** @brief Create a Block from the raw buffer
   */
  explicit
  BlockN(const ConstBufferPtr& buffer);

  /** @brief Create a Block from a buffer, directly specifying boundaries
   *  of the block within the buffer
   */
  BlockN(const ConstBufferPtr& buffer,
          const Buffer::const_iterator& begin, const Buffer::const_iterator& end);

  /** @brief Create a Block from an array with capacity @p length
   */
  BlockN(const uint8_t* array, size_t length);

  /** @brief Create a Block and allocate buffer with capacity @p capacity
   */
  BlockN(size_t capacity);

  //To do: destructor here
	
public: //basic functions
  /** @brief Allocate a buffer and create a Block from the raw buffer with @p usedsize bytes used
   */
  BlockN*
  allocate(size_t capacity);

  /** @brief Check if the Block is empty
   */
  bool
  hasBuffer() const;

  /** @brief Check if the Buffer is empty
   */
  bool
  empty() const;

  /** @brief Reset this Block
   */
  void
  reset();

  Buffer::const_iterator
  begin() const;
	
  Buffer::const_iterator
  end() const;

  const uint8_t*
  bufferValue() const;

  size_t
  capacity() const;

  size_t
  size() const;

  size_t
  offset() const;

  BlockN*
  next() const;

  /** @brief Get underlying buffer
   */
  shared_ptr<const Buffer>
  getBuffer() const;

  /** @brief Check whether the position @p position is in current block
   */
  bool
  inBlock(size_t position);

  /** @brief Deallocate this block and the underlying buffer 
   */
  void
  deAllocate();

  /** @brief Set pointer to next block as NULL
   */
  void
  setNextNull();

  /** @brief Set pointer to next block as @p block
   */
  void
  setNext(BlockN* block);

  void
  setBegin(Buffer::const_iterator newBegin);

  void
  setSize(size_t size);

  void
  setOffset(size_t offset);

  void
  setCapacity(size_t capacity);
	
public: // EqualityComparable concept
  bool
  operator==(const BlockN& other) const;
	
  bool
  operator!=(const BlockN& other) const;
	
public: // ConvertibleToConstBuffer
  operator boost::asio::const_buffer() const;

private:
  shared_ptr<const Buffer> m_buffer;      //points to a segment of underlying memory
  BlockN* m_next;                          //points to the next block in the wire

  Buffer::const_iterator m_begin; 
  Buffer::const_iterator m_end;
	
  size_t m_capacity;                      //maximum byte size of the buffer
  size_t m_offset;                        //absolute offset in the wire
  //uint32_t m_type;                      //type of this buffer
  size_t m_size;                          //used byte size of the buffer

};
}

#endif // NDN_ENCODING_BLOCK_TEST_HPP

