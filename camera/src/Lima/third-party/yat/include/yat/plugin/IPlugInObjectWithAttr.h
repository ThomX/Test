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

#ifndef _YAT_IPLUGINOBJECT_WITH_ATTR_H_
#define _YAT_IPLUGINOBJECT_WITH_ATTR_H_

#include <yat/plugin/IPlugInObject.h>
#include <yat/plugin/PlugInTypes.h>

namespace yat
{

// ============================================================================
//! \class IPlugInObjectWithAttr 
//! \brief The plugin object with attributes interface.
//!
//! This class is a base class for a plugin object with attributes providing
//! access these attributes.
//! This abstract class can not be used as this and must be derived.
//!
//! Inherits from yat::IPlugInObject class.
// ============================================================================
class YAT_DECL IPlugInObjectWithAttr : public yat::IPlugInObject
{
public:
  //! \brief Enumerates the object's attributes.
  //! \param[out] list List of attributes.
  virtual void enumerate_attributes( yat::PlugInAttrInfoList& list) const
    throw (yat::Exception) = 0;

  //! \brief Enumerates the object's properties.
  //! \param[out] prop_infos List of properties.
  virtual void enumerate_properties( yat::PlugInPropInfos& prop_infos ) const
    throw (yat::Exception) = 0;
  
  //! \brief Sets the object's properties.
  //! \param[in] prop_values List of properties to set.
  virtual void set_properties( yat::PlugInPropValues& prop_values )
    throw (yat::Exception) = 0;
};

} // namespace

#endif
