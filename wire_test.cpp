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

#include "wire_test.hpp"
#include "buffer-stream.hpp"

namespace ndn {

typedef shared_ptr<const Wire>              ConstWirePtr;
typedef shared_ptr<Wire>                    WirePtr;


Wire::Wire()
{
}

Wire::Wire(size_t capacity)
{
  BlockN block(capacity);
  m_begin = &block;
  m_end = m_begin;
  m_current = m_begin;
  m_capacity = capacity;
  m_position = 0;
  m_count = 1;
}

Wire::Wire(BlockN* block)
  :m_begin(block),
  m_current(m_begin),
  m_end(m_begin),
  m_capacity(block->capacity()),
  m_position(block->size())
{
  m_count = 1;
}

bool
Wire::hasWire() const
{
  return static_cast<bool>(m_begin);
}

Wire*
Wire::copy()
{
  if(hasWire()) {
    m_count++;
	return this;
  }
  else
	BOOST_THROW_EXCEPTION(Error("Wire is empty"));
}

size_t 
Wire::position() const
{
  if (!hasWire())
	BOOST_THROW_EXCEPTION(Error("Wire is empty"));

  return m_position;
}

size_t 
Wire::capacity() const
{
  if (!hasWire())
	BOOST_THROW_EXCEPTION(Error("Wire is empty"));

  return m_capacity;
}

size_t 
Wire::size() const
{
  if (!hasWire())
	BOOST_THROW_EXCEPTION(Error("Wire is empty"));

  return m_end->offset()+ m_end->size();
}


void
Wire::setPositon(size_t position)
{
  if (!hasWire())
	BOOST_THROW_EXCEPTION(Error("Wire is empty"));
	
  if(position <= size()) {
    // Is the new position within the current memory block?
    if (m_current->inBlock(position)) {
      // we're ok, new position is in this buffer, we're done :)
    } 
	else {
      // we need to find the right buffer
      BlockN *block = m_begin;
      while (!block->inBlock(position)) {
        block = block->next();
      }
      m_current = block;
	}
  }
  m_position = position;
}

BlockN*
Wire::findPosition(Buffer::const_iterator& begin, size_t position) const
{
  if (!hasWire())
    BOOST_THROW_EXCEPTION(Error("Wire is empty"));

  BlockN *block = m_begin;
  if(position <= size()) {
	// Is the new position within the current memory block?
    if (m_current->inBlock(position)) {
      // we're ok, new position is in this buffer, we're done :)
    } 
	else {
      // we need to find the right buffer
      while (!block->inBlock(position)) {
        block = block->next();
      }
    }
  }
  size_t relativeOffset = position - block->offset();
  begin = block->begin() + relativeOffset;
  return block;
}

uint32_t
Wire::type() const
{
  return m_type;
}

size_t 
Wire::setIovec()
{
  size_t iovcnt = countBlock();
  size_t totalSize = 0;

  BlockN *block = m_begin;
  for (int i = 0; i < iovcnt; i++) {
	totalSize += block->size();
    m_iovec.push_back(block->getBuffer());
    block = block->next();
  }
  return totalSize;
}

size_t
Wire::countBlock()
{
  size_t count = 0;
  BlockN *block = m_begin;
  while (block) {
	count++;
	block = block->next();
  }
  return count;
}

void 
Wire::finalize()
{
  // if we're at the limit, we're done
  if (m_position < size()) {
    // begin at the tail and free memory blocks until we've found the current position
    size_t position = m_position;

    // Is the new position within the current memory block?
    if (m_current->inBlock(position)) {
      // we're ok, new position is in this buffer, we're done :)
    } 
    else {
      // we need to find the right buffer
      BlockN *block = m_begin;
      while (!block->inBlock(position)) {
        block = block->next();
      }
      m_current = block;
    }
    // discard any memory blocks after this
    BlockN *current = m_current->next();
    while (current) {
      BlockN *next = current->next();
      current->setNextNull();
      current->deAllocate();     //still some problems here
      current = next;
    }
    // Set the limit of the current block so buffer->position is the end
    m_current->setNextNull();
    size_t setSize = m_position - m_current->offset();
    m_current->setSize(setSize);
    m_end = m_current;
  }
}

bool
Wire::hasIovec()
{
  return static_cast<bool>(m_iovec.size());
}

shared_ptr<Buffer>
Wire::getBufferFromIovec() //still some problems here
{
  if(!hasIovec())
  	BOOST_THROW_EXCEPTION(Error("The iovec is empty")); //if iovec is not constructed, it fails
  OBufferStream os;
  for (io_iterator i = m_iovec.begin(); i != m_iovec.end(); ++i) {
  os.write(reinterpret_cast<const char*>(i->begin()), i->size());
}
  return os.buf();
}

size_t
Wire::remainingInCurrentBlock()
{
  size_t remaining = m_current->offset() + m_current->capacity() - m_position;
  if(remaining < 0)
	BOOST_THROW_EXCEPTION(Error("Position of current block gets wrong"));
  return remaining;
}

void
Wire::expand(size_t allocationSize)
{
  BlockN b;
  BlockN *block =b.allocate(allocationSize);
  m_capacity += block->capacity();
  block->setBegin(m_end->begin() + m_end->size());
	
  m_end->setNext(block);
  m_end->setCapacity(m_end->size());  //tailor the capacity of the last block into its current size
  m_end = block;
}

void
Wire::expandIfNeeded()
{
  if (m_position == m_current->offset()+ m_current->capacity()) {
	if (m_current->next()) {
	  m_current = m_current->next();
	} 
	else {
      //it's the end of the wire
	  expand(2048);
	  m_current = m_end;
	}
  }
}

void
Wire::reserve(size_t length)
{
  /*If the current block has a next pointer, then the remaining is from 
   *the position to its size. Otherwise it is from the position to the end.
   */
  size_t remaining;
  remaining = m_current->offset() + m_current->capacity() - m_position;
	
  if (remaining < length) {
    // if remaining space of this block is small, just finalize it and allocate a new one
    // need to guarantee the remaining space is enough at least for T and L 
    // specific number needs to be considered again
    if (remaining < 32 && m_current->next() == NULL) {
      expand(2048);
      m_current = m_end;
      return;
    }
    // otherwise, use the remaining sapce in current buffer and allocate a new one
    expandIfNeeded();
  }
}

size_t 
Wire::writeUint8(uint8_t value)
{
  expandIfNeeded();
	
  size_t relativeOffset = m_position - m_current->offset();
  Buffer::const_iterator iter = m_current->begin() + relativeOffset + 1;
  *iter = value;
  if (relativeOffset > m_current->size()) {
	m_current->setSize(relativeOffset);
  }

  m_position++;
  return 1;
}

size_t 
Wire::writeUint16(uint16_t value)
{
  reserve(2);

  writeUint8(value >> 8);
  writeUint8(value & 0xFF);
  return 2;
}

size_t 
Wire::writeUint32(uint32_t value)
{
  reserve(4);

  for (int i = sizeof(uint32_t) - 1; i > 0; i--) {
    uint8_t byte = value >> (i * 8) & 0xFF;
    writeUint8(byte);
  }

  writeUint8(value & 0xFF);
  return 4;
}

size_t 
Wire::writeUint64(uint64_t value)
{
  reserve(8);

  for (int i = sizeof(uint64_t) - 1; i > 0; i--) {
    uint8_t byte = value >> (i * 8) & 0xFF;
    writeUint8(byte);
  }

  writeUint8(value & 0xFF);
  return 8;
}

size_t 
Wire::appendArray(const uint8_t* array, size_t length)
{
  expandIfNeeded();
	
  size_t offset = 0;
  while (offset < length) {
    size_t remaining = remainingInCurrentBlock();
    if (remaining == 0) {
      expandIfNeeded();
    } 
    else {
      if (remaining > (length - offset)) {
        remaining = length - offset;
      }
	
      size_t relativeOffset = m_position - m_current->offset();
      auto dest = m_current->begin() + relativeOffset;
      auto src = array + offset;     //notice !!
      std::copy(src, src + remaining, dest);
	
	  relativeOffset += remaining;
	  if (relativeOffset > m_current->size()) { 
	    m_current->setSize(relativeOffset);
	  }
	
	  m_position += remaining;
	  offset += remaining;
    }
  }
  return length;
}

size_t 
Wire::appendBlock(BlockN* block)
{
  finalize();
  // we assume that this is a single block (only when a block is put into a wire it will has next pointer)
  if (!(block->next()))
    BOOST_THROW_EXCEPTION(Error("block does not have next pointer until put into a wire"));

  m_end->setNext(block);
  m_end = block;
  m_end->setOffset(m_position);

  m_current = m_end; 
  m_position += m_end->size();
  m_capacity += m_end->capacity();

  return m_end->size();
}


uint8_t 
Wire::readUint8(size_t position) const
{
  if(m_capacity >position) {
    BlockN *block = m_begin;
    while(block && !block->inBlock(position)) {
      block = block->next();
    }
    size_t relativeOffset = position - block->offset();
    return *(block->begin() + relativeOffset);
  }
  else
    BOOST_THROW_EXCEPTION(Error("could not find the illegal position"));
}

shared_ptr<Buffer>
Wire::getBuffer()
{
  OBufferStream os;
  size_t totalSize = 0;
  BlockN *block = m_begin;
  while (block) {
  	totalSize += block->size();
    os.write(reinterpret_cast<const char*>(block->bufferValue()), block->size());
    block = block->next();
  }
  return os.buf();
}

void
Wire::parse() const
{
  if (!m_subWires.empty() || size() == 0)	//there have been some wires in the container
    return;
	
  size_t begin = 0;
  size_t end = size();
	
  while (begin != end) {
    size_t element_begin = begin;
	Buffer::const_iterator tmp_begin;
	
	uint32_t type = tlv::readType(*this, begin, end);
	uint64_t length = tlv::readVarNumber(*this, begin, end);
	
	if (length > static_cast<uint64_t>(end - begin)) {
	  m_subWires.clear();				//********************
	  BOOST_THROW_EXCEPTION(tlv::Error("TLV length exceeds buffer length"));
        }
	size_t element_end = begin + length;
	Buffer::const_iterator tmp_end;
	BlockN* beginBlock = findPosition(tmp_begin, element_begin);
	BlockN* endBlock = findPosition(tmp_end, element_end);
	//if subwire's begin and subwire's end are in the same block(underlying buffer is consecutive)
	if (beginBlock = endBlock) {
          BlockN* first = new BlockN(beginBlock->getBuffer(), tmp_begin, tmp_end);
	  Wire wire = Wire(first);
	  wire.m_type = type;
	  m_subWires.push_back(wire); 
	}
	else {
          BlockN* first = new BlockN(beginBlock->getBuffer(), tmp_begin, tmp_end);
          Wire wire = Wire(first);
          wire.m_type = type;
	  BlockN* block= beginBlock->next();
	  while(block != endBlock) { 
	  	wire.appendBlock(BlockN(block->getBuffer(), block->begin(), block->end())); 
		block = block->next();
	  }
	  wire.appendBlock(BlockN(block->getBuffer(), block->begin(), tmp_end()));
	  m_subWires.push_back(wire);
        }
	begin = element_end;
	// don't do recursive parsing, just the top level
  }
}

const Wire&
Wire::get(uint32_t type) const
{
  element_const_iterator it = this->find(type);
  if (it != m_subWires.end())
    return *it;
	
  BOOST_THROW_EXCEPTION(Error("(Wire::get) Requested a non-existed type [" +
							  boost::lexical_cast<std::string>(type) + "] from Wire"));
}

Wire::element_const_iterator
Wire::find(uint32_t type) const  
{
  return std::find_if(m_subWires.begin(), m_subWires.end(),
                      [type] (const Wire& subWire) { return subWire.type() == type; });
}

const Wire::element_container&
Wire::elements() const
{
  return m_subWires;
}

Wire::element_const_iterator
Wire::elements_begin() const
{
  return m_subWires.begin();
}

Wire::element_const_iterator
Wire::elements_end() const
{
  return m_subWires.end();
}

size_t
Wire::elements_size() const
{
  return m_subWires.size();
}

}



