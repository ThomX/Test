//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2012  N.Leclercq & The Tango Community
//
// Part of the code comes from the ACE Framework (asm bytes swaping code)
// see http://www.cs.wustl.edu/~schmidt/ACE.html for more about ACE
//
// The thread native implementation has been initially inspired by omniThread
// - the threading support library that comes with omniORB. 
// see http://omniorb.sourceforge.net/ for more about omniORB.
//
// Contributors form the TANGO community:
// See AUTHORS file 
//
// The YAT library is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
//
// The YAT library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// See COPYING file for license details 
//
// Contact:
//      Nicolas Leclercq
//      Synchrotron SOLEIL
//------------------------------------------------------------------------------
/*!
 * \author See AUTHORS file
 */

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include <yat/bitsstream/Endianness.h>
#include <yat/bitsstream/BitsStream.h>

namespace yat 
{

//=============================================================================
// Global var. for BitsStream
//=============================================================================
YAT_DECL size_t kINDENT_COUNTER = 0;

//=============================================================================
// BitsStream::BitsStream
//=============================================================================
BitsStream::BitsStream (unsigned char * _data, 
                        size_t _size, 
                        const Endianness::ByteOrder& _endianness)
 : m_current_byte (0),
   m_bits_in_current_byte (0),
   m_ibuffer (_data),
   m_ibuffer_size (_size),
   m_ibuffer_ptr (0),
   m_endianness (_endianness)
{
  //- noop ctor
}

//=============================================================================
// BitsStream::~BitsStream
//=============================================================================
BitsStream::~BitsStream ()
{
  //- noop dtor
}

/*
//-------------------------------------------------------------------------
  BACKUP OF THE VERBOSE VERSION OF <read_bits_i> FOR DEBUG PURPOSE
//-------------------------------------------------------------------------
inline bool read_bits_i (int _num_bits, yat::BitsStorage * _bits)
{
  bool retval = true;
      
  std::cout << "-------------------------------------" << std::endl;
  std::cout << "> initial bits............." << *_bits << std::endl;
  std::cout << "> initial num_bits........." << _num_bits << std::endl;
  binary_dump("current-byte", m_current_byte);
  
  if ( m_ibuffer != 0 ) 
  {
    //- this while loop extracts full bytes
    while ( _num_bits > m_bits_in_current_byte )
    {
      std::cout << "> num_bits > bits_in_current_byte" << std::endl;

      if ( _bits )
      {
        int shift = _num_bits - m_bits_in_current_byte;
        std::cout << "> doing: *bits |= m_current_byte << " << shift << std::endl; 
        binary_dump("bits", *_bits);
        *_bits |= m_current_byte << shift;
        binary_dump("shifted-current-byte", m_current_byte << shift);
        binary_dump("bits", *_bits);
      }
      
      std::cout << "- num_bits................." << _num_bits << std::endl;
      std::cout << "- bits_in_current_byte....." << m_bits_in_current_byte << std::endl;
      _num_bits -= m_bits_in_current_byte;
      std::cout << "> consumed " << m_bits_in_current_byte << " bits of the current byte" << std::endl; 
      std::cout << "- num_bits................." << _num_bits << std::endl;

      if ( m_ibuffer_ptr != m_ibuffer_size )
      {
        std::cout << "> moving to next byte in buffer" << std::endl;
        m_current_byte = m_ibuffer[m_ibuffer_ptr++];
        binary_dump("current-byte", m_current_byte);
        m_bits_in_current_byte = 8;
        std::cout << "- bits_in_current_byte....." << m_bits_in_current_byte << std::endl;
      }
      else
      {
        std::cout << "> reached end of buffer" << std::endl;
        m_bits_in_current_byte = 0;
        m_current_byte = 0;
        retval = false;
        break;
      }
    }

    if ( _num_bits > 0 && retval )
    {
      std::cout << "> num_bits > 0 && retval is true" << std::endl;
      
      if ( _bits )
      {
        yat::BitsStorage bit_mask = 0x0001;
        for ( int i = 1; i < _num_bits; i++ )
          bit_mask = ( bit_mask << 1 ) | 0x0001;

        std::cout << "> doing: *bits |= current_byte & bit_mask" << std::endl; 
        binary_dump("current-byte", m_current_byte);
        binary_dump("bit_mask", bit_mask);
        binary_dump("current_byte & bit_mask", m_current_byte & bit_mask);
        binary_dump("bits", *_bits);
        *_bits |= m_current_byte & bit_mask;
        binary_dump("bits", *_bits);
      }
      
      std::cout << "> doing: current_byte = current_byte >> " << _num_bits << std::endl; 
      m_current_byte = m_current_byte >> _num_bits;
      binary_dump("current-byte", m_current_byte);
        
      std::cout << "> doing: bits_in_current_byte -= num_bits" << std::endl; 
      std::cout << "- num_bits................." << _num_bits << std::endl;
      std::cout << "- bits_in_current_byte....." << m_bits_in_current_byte << std::endl;
      
      m_bits_in_current_byte -= _num_bits;
      
      std::cout << "- bits_in_current_byte....." << m_bits_in_current_byte << std::endl;
    }
  }

  std::cout << "> returning " << retval << std::endl;

  return retval;
}
*/

} //- namespace



