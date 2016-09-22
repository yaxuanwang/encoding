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

#ifndef NDN_ENCODING_WIRE_TEST_HPP 
#define NDN_ENCODING_WIRE_TEST_HPP
 
#include "block_test.hpp"
#include "../common.hpp"
#include "tlv_test.hpp"

     
namespace boost {
namespace asio {
class const_buffer;
} // namespace asio
} // namespace boost

namespace ndn {
/** @brief Class representing a series of linked blocks
 */
class Wire
{
public:
  typedef std::vector<shared_ptr<const Buffer>>     io_container;
  typedef io_container::iterator              io_iterator;
  typedef io_container::const_iterator        io_const_iterator;
	
  typedef std::vector<Wire>                   element_container;
  typedef element_container::iterator         element_iterator;
  typedef element_container::const_iterator   element_const_iterator;

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

public://constructor
  /** @brief Create an empty wire
   */
  Wire();
	
  /** @brief Create the first block in wire with capacity @p capacity
   */
  Wire(size_t capacity);
	
  /** @brief Create a wire with the fisrt block @p block
   */
  Wire(BlockN* block);

  /** @brief Create a wire with the fisrt block whose buffer is @p buffer
   *  @param begin the begin of data in this buffer
   *  @param begin the end of data in this buffer
   */
  Wire(BufferPtr& buffer, Buffer::const_iterator begin, Buffer::const_iterator end);
  
public: //wire
  /** @brief Check if the Wire is empty
   */
  bool
  hasWire() const;
	
  /** @brief Increase a reference to current wire
   */
  Wire*
  copy();
	
  /** @brief Return current offset in this wire
   */
  size_t 
  position() const;
	
  /** @brief Return current capacity of this wire
   */
  size_t 
  capacity() const;
	
  /** @brief Return current used byte size of this wire
   */
  size_t 
  size() const;
	
  /** @brief Set the offset to a specific positon
   */
  void
  setPositon(size_t position);

  /** @brief Find the block and buffer which @p position offset lies, set the iterator to this position
   *  Return the pointer to this block
   */
  BlockN*
  findPosition(Buffer::const_iterator& begin, size_t position) const;

  uint32_t
  type() const;
		
public: //iovec
  /** @brief put buffers pointed by blocks into a buffer sequence iovec
   *  All buffers in blocks in this wire will not be copied or modified. The iovec is read-only for doing a gathering write.
   *  Return total byte size
   */
  size_t 
  setIovec();
	
  /** @brief count the number of blocks in this wire
   */
  size_t
  countBlock();
	
  /** @brief set the size of this wire to the current postion and throw ohters  
   */
  void 
  finalize(); 
	
  /** @brief linerize the wire into a single buffer  
   */
  shared_ptr<Buffer>
  getBufferFromIovec();
	
  /** @brief check if there is element in iovec  
   */
  bool
  hasIovec();

public: //operate the wire
  /** @brief Return the remaining bytes space between positon and capacity of current block 
   */
  size_t
  remainingInCurrentBlock();
	
  /** @brief Expand the wire with a new block adding to the end with capacity @p allocationSize  
   *  Defualt size is 2048
   */
  void
  expand(size_t allocationSize);
	
  /** @brief Expand the wire when current capacity is not enough  
   */
  void
  expandIfNeeded();
	
  /** @brief Check if there are @p length bytes left in a single block's contiguous underlying memory.
   *  If not, and the remaining buffer space is small, we will finalize it and allocate a new block.
   */
  void
  reserve(size_t length);
	
  /** @brief Write a `uint8_t` to the current position, allocating as necessary  
   */
  size_t 
  writeUint8(uint8_t value);
	
  /** @brief Write a `uint16_t` to the current position, allocating as necessary  
   */
  size_t 
  writeUint16(uint16_t value);
	
  /** @brief Write a `uint32_t` to the current position, allocating as necessary  
   */
  size_t
  writeUint32(uint32_t value);
	
  /** @brief Write a `uint64_t` to the current position, allocating as necessary  
   */
  size_t 
  writeUint64(uint64_t value);
	
  /** @brief Append an array with @p length bytes to the current position, allocating new block as necessary
   *  Return the size of the appended array
   */
  size_t 
  appendArray(const uint8_t* array, size_t length);
	
  /** @brief Append a block to the current position 
   *  This will call finalize and throw buffer after current position
   *  Return the size of the appended block
   */
  size_t 
  appendBlock(BlockN* block);
	
  /** @brief Insert a block to the current position 
   *  This will throw data in current block after current position
   */
  size_t 
  insertBlock(const BlockN* block); //complicated 

  /** @brief Append a wire @p wire to the current position (not decided yet)
   *  This function will combine two wire together into a longer one
   */
  size_t 
  appendWire(const Wire* wire);

  /** @brief read the `uint8_t` in @p position position 
   */
  uint8_t 
  readUint8(size_t position) const;

  /** @brief From logical continuous wire create a physical continuous memory buffer
   *  Return the shared pointer of this underlying buffer
   */
  shared_ptr<Buffer>
  getBuffer();

public: //subwires
  /** @brief Parse this wire into subwires
   *
   *  This method will not copy or modify any data.  It simply
   *  parses contents of the wire into subwires in the format of TLVs
   *  It only parses the top level TLVs, do not do recursive parsing
   */
  void
  parse() const;

  /** @brief Get the first subelement of the requested type
   */
  const Wire&
  get(uint32_t type) const;

  /** @brief Find the position of first subelement of the requested type
   */
  element_const_iterator
  find(uint32_t type) const;

  /** @brief Get all subelements
   */
  const element_container&
  elements() const;

  element_const_iterator
  elements_begin() const;

  element_const_iterator
  elements_end() const;

  size_t
  elements_size() const;

private:
  size_t m_position;               //absolute offset in this wire
  size_t m_capacity;               //total maximum byte size of this wire

  BlockN* m_begin;                  //first block
  BlockN* m_current;                //current block
  BlockN* m_end;                    //the last block
  io_container m_iovec;            //buffer sequence
  size_t m_count;                  //reference time(not decided yet) 
  uint32_t m_type;                 //type of this wire
  mutable element_container m_subWires;

};
}        

#endif // NDN_ENCODING_WIRE_TEST_HPP

