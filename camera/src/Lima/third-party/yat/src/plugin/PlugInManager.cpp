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
#include <yat/plugin/PlugInManager.h>

namespace yat
{

//==============================================================================
// PlugInManager::PlugInManager
//==============================================================================
PlugInManager::PlugInManager()
{
}

//==============================================================================
// PlugInManager::~PlugInManager
//==============================================================================
PlugInManager::~PlugInManager()
{
  unload_all();
}

//==============================================================================
// PlugInManager::load
//==============================================================================
std::pair<IPlugInInfo*, IPlugInFactory*>
  PlugInManager::load( const std::string &library_file_name )
{
  PlugInEntry entry;
  load(library_file_name, &entry);
  return std::make_pair(entry.m_info , entry.m_factory);
}

//==============================================================================
// PlugInManager::load (new version)
//==============================================================================
void PlugInManager::load( const std::string &library_file_name, 
                          PlugInManager::PlugInEntry* entry_ptr )
{
  if( !entry_ptr )
      THROW_YAT_ERROR("NULL_POINTER",
                      "Caller must pass a valid pointer to PlugInManager::PlugInEntry structure",
                      "PlugInManager::load");

  //- first verify that the plugin is not already loaded
  for( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
  {
    if( (*it).m_fileName == library_file_name )
    {
      // Already loaded !
      *entry_ptr = *it;
      return;
    }
  }

  //- ok, does not already exist : load it
  (*entry_ptr).m_fileName = library_file_name;
  (*entry_ptr).m_plugin = new PlugIn( library_file_name );
  (*entry_ptr).m_info = (*entry_ptr).m_plugin->info();
  (*entry_ptr).m_factory = (*entry_ptr).m_plugin->factory();

  m_plugIns.push_back( *entry_ptr );
}

//==============================================================================
// PlugInManager::unload
//==============================================================================
void PlugInManager::unload( const std::string &library_file_name )
{
  if( m_plugIns.empty() )
    return;
  for( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
  {
    if ( (*it).m_fileName == library_file_name )
    {
      unload( *it );
      m_plugIns.erase( it );
      break;
    }
  }
}

//==============================================================================
// PlugInManager::unload_all
//==============================================================================
void PlugInManager::unload_all( void )
{
  if( m_plugIns.empty() )
    return;
  for( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
  {
    unload( *it );
  }
}

//==============================================================================
// PlugInManager::unload
//==============================================================================
void PlugInManager::unload( PlugInEntry &plugin_info )
{
  try
  {
    delete plugin_info.m_factory;
    delete plugin_info.m_info;
    delete plugin_info.m_plugin;
  }
  catch (...)
  {
    plugin_info.m_plugin = NULL;
    throw;
  }
}

}
