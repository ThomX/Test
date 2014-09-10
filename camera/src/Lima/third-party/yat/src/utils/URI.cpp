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
#include <sstream>
#include <yat/utils/URI.h>

namespace yat
{

const String uri_syntax_error = "BAD_URI_SYNTAX";

//----------------------------------------------------------------------------
// URI::URI (const String& uri_string)
//----------------------------------------------------------------------------
URI::URI(const String& uri_string) throw ( Exception )
{
  parse(uri_string);
}

//----------------------------------------------------------------------------
// URI::URI (const String& scheme, const String& authority...)
//----------------------------------------------------------------------------
URI::URI(const URI::Fields& fields) throw ( Exception )
{
  check(URI::SCHEME, fields.scheme);
  check(URI::USERINFO, fields.userinfo);
  check(URI::HOST, fields.host);
  check(URI::PORT, fields.port);
  check(URI::PATH, fields.path);
  check(URI::QUERY, fields.query);
  check(URI::FRAGMENT, fields.fragment);
  
  m_part[URI::SCHEME]    = fields.scheme;
  m_part[URI::USERINFO]  = fields.userinfo;
  m_part[URI::HOST]      = fields.host;
  m_part[URI::PORT]      = fields.port;
  m_part[URI::PATH]      = fields.path;
  m_part[URI::QUERY]     = fields.query;
  m_part[URI::FRAGMENT]  = fields.fragment;
}

const String ALPHA = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const String DIGIT = "0123456789";
const String UNRESERVED = ALPHA + DIGIT + String("-._~");
const String HEXDIGIT ="0123456789abcdefABCDEF";
const String PCT_ENCODED  = String("%") + HEXDIGIT;
const String SUB_DELIMS = "!$&'()*+,;=";
const String GEN_DELIMS = ":/?#[]@";

//----------------------------------------------------------------------------
// URI::check_value
//----------------------------------------------------------------------------
bool URI::check_value(const String& value, const String &accepted_chars, const String& value_name, bool throw_exception) throw ( Exception )
{
  if( value.find_first_not_of(accepted_chars) != std::string::npos )
  {
    if( throw_exception )
      throw Exception(uri_syntax_error, String::str_format("Bad '%s' syntax: %s.", PSZ(value_name), PSZ(value)), "URI::check_value");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
// URI::check 
//----------------------------------------------------------------------------
bool URI::check(URI::Part part, const String& value, bool throw_exception) throw ( Exception )
{
  switch ( part )
  {
    case URI::SCHEME:
      return check_value(value, ALPHA + DIGIT + "+-.", PSZ_FMT("Bad scheme syntax: %s.", PSZ(value)), throw_exception);
    
    case URI::AUTHORITY:
    {
      URI::Fields fields;
      return check_authority(value, &fields, throw_exception);
    }
      
    case URI::USERINFO:
      return check_value(value, UNRESERVED + PCT_ENCODED + SUB_DELIMS + ":", "userinfo", throw_exception);
      
    case URI::HOST:
      return check_value(value, UNRESERVED + PCT_ENCODED + SUB_DELIMS + ":" + "[]", "host", throw_exception);
      
    case URI::PORT:
      return check_value(value, DIGIT, "port", throw_exception);

    case URI::PATH:
      return check_value(value, UNRESERVED + PCT_ENCODED + SUB_DELIMS + ":@", "host", throw_exception);
      return true;

    case URI::QUERY:
      return check_value(value, UNRESERVED + PCT_ENCODED + SUB_DELIMS + ":@/?", "query", throw_exception);
      
    case URI::FRAGMENT:
      return check_value(value, UNRESERVED + PCT_ENCODED + SUB_DELIMS + ":@/?", "fragment", throw_exception);
  }

  if( throw_exception )
    throw Exception(uri_syntax_error, "invalid URI::Part specified", "URI::check_value");
  
  return false;
}

//----------------------------------------------------------------------------
// URI::split_authority
//----------------------------------------------------------------------------
void URI::split_authority(const String& authority, String* userinfo_ptr, String* host_ptr, String* port_ptr)
{
  if( !authority.empty() )
  {
    String value = authority;
    
    // Look for user info
    String userinfo;
    if( value.find('@') != std::string::npos )
      value.extract_token('@', userinfo_ptr);
    
    // Look for port number
    String port;
    if( value.find(':') != std::string::npos )
      value.extract_token_right(':', port_ptr);
    
    // The remaining part is the host name
    *host_ptr = value;
  }
}

//----------------------------------------------------------------------------
// URI::check_authority
//----------------------------------------------------------------------------
bool URI::check_authority(const String& authority, URI::Fields* fields_ptr, bool throw_exception) throw ( Exception )
{
  split_authority(authority, &(fields_ptr->userinfo), &(fields_ptr->host), &(fields_ptr->port));
  if( fields_ptr->host.empty() )
  {
    if( throw_exception )
      throw Exception(uri_syntax_error, "'host' part must not be empty!", "URI::check_authority");
    return false;
  }
  if( !check(URI::USERINFO, fields_ptr->userinfo, throw_exception) )
    return false;
  if( !check(URI::HOST, fields_ptr->host, throw_exception) )
    return false;
  if( !check(URI::PORT, fields_ptr->port, throw_exception) )
    return false;
  
  return true;
}

//----------------------------------------------------------------------------
// URI::parse 
//----------------------------------------------------------------------------
void URI::parse(const String& uri) throw ( Exception )
{
  URI::Fields fields;
  String all = uri, authority, query, fragment, userinfo, host, port;
  
  // Scheme part is mandatory
  all.extract_token(':', &fields.scheme);
  fields.scheme.to_lower();
  check(URI::SCHEME, fields.scheme, true);
  
  // Hierarchical part (authority or path) is mandatory
  // This part ends with first '?' or '#'
  String hierarchical_part;
  char found_sep = 0;
  if( all.find('?') != std::string::npos )
  {
    all.extract_token('?', &hierarchical_part);
    found_sep = '?';
  }
  else if(  all.find('#') != std::string::npos )
  {
    all.extract_token('#', &hierarchical_part);
    found_sep = '#';
  }
  else
    hierarchical_part = all;
  
  if( hierarchical_part.empty() )
    throw Exception(uri_syntax_error, "Hierarchical part (authority and/or path) must not be empty!", "URI::parse");
  
  if( hierarchical_part.match("//*") )
  {
    // There is an authority
    hierarchical_part = hierarchical_part.substr(2);
    String authority;
    hierarchical_part.extract_token('/', &authority);
    check_authority(authority, &fields, true);
    
    // Last part is the path
    fields.path = "/" + hierarchical_part;
  }
  else
  {
    fields.path = hierarchical_part;
  }

  check(URI::PATH, fields.path);
  
  if( '?' == found_sep )
  {
    // Extract query part
    if(  all.find('#') != std::string::npos )
      all.extract_token_right('#', &fields.fragment);
    fields.query = all;
    check(URI::QUERY, fields.query);
  }
  else if( '#' == found_sep )
  {
    fields.fragment = all;
    check(URI::FRAGMENT, fields.fragment);
  }
  
  // Sets the values
  m_part[SCHEME]    = fields.scheme;
  m_part[USERINFO]  = fields.userinfo;
  m_part[HOST]      = fields.host;
  m_part[PORT]      = fields.port;
  m_part[PATH]      = fields.path;
  m_part[QUERY]     = fields.query;
  m_part[FRAGMENT]  = fields.fragment;
}


//----------------------------------------------------------------------------
// URL::get 
//----------------------------------------------------------------------------
String URI::get() const
{
  std::ostringstream s;
  s << value(URI::SCHEME) << ':';
  String authority = get(URI::AUTHORITY);
  if( !authority.empty() )
    s << "//" << authority;
    
  String path = get(URI::PATH);
  if( !path.empty() )
    s << path;
  
  String query = get(URI::QUERY);
  if( !query.empty() )
    s << '?' << query;

  String fragment = get(URI::FRAGMENT);
  if( !fragment.empty() )
    s << '#' << fragment;
    
  return s.str();
}

//----------------------------------------------------------------------------
// URI::value
//----------------------------------------------------------------------------
String URI::value(URI::Part part) const
{
  std::map<Part, String>::const_iterator cit = m_part.find(part);
  if( cit != m_part.end() )
    return cit->second;
  return String::nil;
}

//----------------------------------------------------------------------------
// URI::get
//----------------------------------------------------------------------------
String URI::get(URI::Part part) const
{
  if( URI::AUTHORITY == part )
  {
    std::ostringstream s;
    if( !value(URI::USERINFO).empty() )
      s <<  value(URI::USERINFO) << '@';
    if( !value(URI::HOST).empty() )
      s <<  value(URI::HOST);
    if( !value(URI::PORT).empty() )
      s <<  ':' << value(URI::PORT);
    
    return s.str();
  }
  else
  {
    return value(part);
  }  
}

//----------------------------------------------------------------------------
// URI::set
//----------------------------------------------------------------------------
void URI::set(URI::Part part, const String &value) throw ( Exception )
{
  if( URI::AUTHORITY == part )
  {
    URI::Fields fields;
    check_authority(value, &fields, true);
    m_part[URI::USERINFO] = fields.userinfo;
    m_part[URI::HOST] = fields.host;
    m_part[URI::PORT] = fields.port;
  }
  else
  {
    check(part, value, true);
    m_part[part] = value;
  }
}

//----------------------------------------------------------------------------
// URI::set
//----------------------------------------------------------------------------
void URI::set(const String &value) throw ( Exception )
{
  parse(value);
}

} //- namespace yat

