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

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat/network/ClientSocket.h>

#if !defined (YAT_INLINE_IMPL)
# include <yat/network/ClientSocket.i>
#endif // YAT_INLINE_IMPL


namespace yat {

// ----------------------------------------------------------------------------
// ClientSocket::ClientSocket
// ----------------------------------------------------------------------------
ClientSocket::ClientSocket (Socket::Protocol _p) 
  : Socket(_p),
    m_connection_status(ClientSocket::CONNECTED_NO)
{
  YAT_TRACE("yat::ClientSocket::ClientSocket");
}

// ----------------------------------------------------------------------------
// ClientSocket::~ClientSocket
// ----------------------------------------------------------------------------
ClientSocket::~ClientSocket ()
{
  YAT_TRACE("yat::ClientSocket::~ClientSocket");

  try
  {
    this->close();
  }
  catch (...)
  { 
    //- ignore any error
  }
}

// ----------------------------------------------------------------------------
// ClientSocket::bind
// ----------------------------------------------------------------------------
void ClientSocket::bind (size_t _p)
  throw (SocketException)
{
  YAT_TRACE("yat::ClientSocket::bind");

  this->Socket::bind(_p);
}

// ----------------------------------------------------------------------------
// ClientSocket::connect
// ----------------------------------------------------------------------------
void ClientSocket::connect (const Address & _a)
  throw (SocketException)
{
  YAT_TRACE("yat::ClientSocket::connect");

  try
  {
    this->Socket::connect(_a);
  }
  catch (...)
  {
    this->m_connection_status = ClientSocket::CONNECTED_NO;
    throw;
  }

  this->m_connection_status = ClientSocket::CONNECTED_YES;
}

// ----------------------------------------------------------------------------
// ClientSocket::disconnect
// ----------------------------------------------------------------------------
void ClientSocket::disconnect ()
  throw (SocketException)
{
  YAT_TRACE("yat::ClientSocket::disconnect");

  try
  {
    this->close();
  }
  catch (...)
  {
    this->m_connection_status = ClientSocket::CONNECTED_NO;
    throw;
  }

  this->m_connection_status = ClientSocket::CONNECTED_NO;
}

// ----------------------------------------------------------------------------
// ClientSocket::can_read_without_blocking
// ----------------------------------------------------------------------------
bool ClientSocket::can_read_without_blocking ()
  throw (SocketException)
{
  YAT_TRACE("yat::ClientSocket::can_read_without_blocking");

  return this->select(0);
}

// ----------------------------------------------------------------------------
// ClientSocket::wait_input_data
// ----------------------------------------------------------------------------
bool ClientSocket::wait_input_data (size_t _tmo, bool _throw)
  throw (SocketException)
{
  YAT_TRACE("yat::ClientSocket::wait_input_data");
 
  if ( ! this->select(_tmo) )
  {
    if (! _throw)
      return false;

    throw yat::SocketException("SOCKET_ERROR", 
                               SocketException::get_error_text(SoErr_TimeOut), 
                               "yat::ClientSocket::wait_input_data", 
                               SocketException::yat_to_native_error(SoErr_TimeOut)); 
  } 
  
  return true;
}

} //- namespace yat

