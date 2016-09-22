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

#include "block_test.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/asio/buffer.hpp>

namespace ndn {

#if NDN_CXX_HAVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE  //not to throw any exception.
static_assert(std::is_nothrow_move_constructible<BlockN>::value,
              "Block must be MoveConstructible with noexcept");
#endif // NDN_CXX_HAVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE

#if NDN_CXX_HAVE_IS_NOTHROW_MOVE_ASSIGNABLE
static_assert(std::is_nothrow_move_assignable<BlockN>::value,
              "Block must be MoveAssignable with noexcept");
#endif // NDN_CXX_HAVE_IS_NOTHROW_MOVE_ASSIGNABLE

const size_t MAX_SIZE_OF_BLOCK_FROM_STREAM = MAX_NDN_PACKET_SIZE; 

BlockN::BlockN()   //create an empty block 
{
}

BlockN::BlockN(const ConstBufferPtr& buffer) 
  : m_buffer(buffer)
  , m_begin(m_buffer->begin())
  , m_end(m_buffer->end())
  , m_capacity(m_end - m_begin)
{
  m_next = NULL;
  m_size = buffer->size();
  m_offset = 0;
}

BlockN::BlockN(const ConstBufferPtr& buffer,
               const Buffer::const_iterator& begin, const Buffer::const_iterator& end)
  : m_buffer(buffer)
  , m_begin(begin)
  , m_end(end)
  , m_capacity(m_end - m_begin)
{
  m_next = NULL;
  m_size = buffer->size();
  m_offset = 0;
}

BlockN::BlockN(const uint8_t* array, size_t length) 
{
  m_buffer = make_shared<Buffer>(array, array+length);
  m_begin = m_buffer->begin();
  m_end = m_buffer->end();
  m_size = m_end - m_begin;
  m_capacity = m_size;
  m_next = NULL;
}

BlockN::BlockN(size_t capacity)
{
  shared_ptr<Buffer> buf = new Buffer(capacity);
  m_buffer = buf;
  m_begin = m_buffer->begin();
  m_end = m_buffer->end();
  m_size = m_end - m_begin;
  m_capacity = m_size;
  m_next = NULL;
}

BlockN*
BlockN::allocate(size_t capacity) 
{
  shared_ptr<Buffer> buf = new Buffer(capacity);
  BlockN block(buf);

  return &block;
}

bool
BlockN::hasBuffer() const
{
  return static_cast<bool>(m_buffer);
}

bool
BlockN::empty() const
{
  return m_buffer && (m_size == 0);
}

void
BlockN::reset()
{
  m_buffer.reset(); // reset of the shared_ptr
  m_begin = m_end = Buffer::const_iterator();
  m_capacity = m_offset = m_size = 0;
  m_next = NULL;
}

Buffer::const_iterator
BlockN::begin() const
{
  if (!hasBuffer())
    BOOST_THROW_EXCEPTION(Error("Underlying buffer is empty"));

  return m_begin;
}

Buffer::const_iterator
BlockN::end() const
{
  if (!hasBuffer())
    BOOST_THROW_EXCEPTION(Error("Underlying buffer is empty"));

  return m_end;
}

const uint8_t*
BlockN::bufferValue() const
{
  if (!hasBuffer())
    BOOST_THROW_EXCEPTION(Error("Underlying buffer is empty"));

  return &*m_begin; 
}

size_t
BlockN::capacity() const
{
  if (hasBuffer()) {   
    return m_capacity;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block capacity cannot be determined (undefined block capacity)"));
}

size_t
BlockN::size() const
{
  if (hasBuffer()) {   
    return m_size;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block used size cannot be determined (undefined block used size)"));
}

size_t
BlockN::offset() const
{
  if (hasBuffer()) {	 
    return m_offset;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block used size cannot be determined (undefined block used size)"));
}

BlockN*
BlockN::next() const
{
  if (hasBuffer()) {	   
    return m_next;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block used size cannot be determined (undefined block used size)"));
}

void
BlockN::setNextNull()
{
  m_next = NULL;
}

void
BlockN::setNext(BlockN* block)
{
  m_next = block;
}

void
BlockN::setSize(size_t size)
{
  m_size = size;
}

void
BlockN::setCapacity(size_t capacity)
{
  m_capacity = capacity;
  //To do: resize the buffer?
}

void
BlockN::setOffset(size_t offset)
{
  m_offset = offset;
}

void
BlockN::setBegin(Buffer::const_iterator newBegin)
{
  m_begin = newBegin;
}

shared_ptr<const Buffer>
BlockN::getBuffer() const
{
  return m_buffer;
}

bool
BlockN::inBlock(size_t position)
{
  if (!hasBuffer())
	BOOST_THROW_EXCEPTION(Error("underlying buffer is empty"));

  return (m_offset <= position && position < m_offset+ m_size);
}

void
BlockN::deAllocate()
{
  //there are still some problems about memory
  this->reset();
}

bool
BlockN::operator!=(const BlockN& other) const
{
  return !this->operator==(other);
}

bool
BlockN::operator==(const BlockN& other) const
{
  return this->size() == other.size() &&
         std::equal(this->begin(), this->end(), other.begin());
}

}

