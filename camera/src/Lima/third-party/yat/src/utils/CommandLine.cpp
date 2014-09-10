//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2012  The Tango Community
//
// Part of the code comes from the ACE Framework (i386 asm bytes swaping code)
// see http://www.cs.wustl.edu/~schmidt/ACE.html for more about ACE
//
// The thread native implementation has been initially inspired by omniThread
// - the threading support library that comes with omniORB. 
// see http://omniorb.sourceforge.net/ for more about omniORB.
//
// Contributors form the TANGO community:
// Ramon Sune (ALBA) for the yat::Signal class 
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
 * \author S.Poirier - Synchrotron SOLEIL
 */

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include <yat/utils/CommandLine.h>

namespace yat
{

//----------------------------------------------------------------------------
// CommandLine::find_short_opt
//----------------------------------------------------------------------------
CommandLine::CommandLineOpt *CommandLine::find_short_opt(const char cOpt)
{
  vecOpts::iterator itOpt;
  for( itOpt = m_vecOptDefs.begin(); itOpt != m_vecOptDefs.end(); itOpt++ )
  {
    if( (*itOpt).cShortName == cOpt )
      return &(*itOpt);
  }
  return NULL;
}

//----------------------------------------------------------------------------
// CommandLine::find_long_opt
//----------------------------------------------------------------------------
CommandLine::CommandLineOpt *CommandLine::find_long_opt(const String &strOpt)
{
  vecOpts::iterator itOpt;
  for( itOpt = m_vecOptDefs.begin(); itOpt != m_vecOptDefs.end(); itOpt++ )
  {
    if( (*itOpt).strLongName.is_equal( strOpt ) )
      return &(*itOpt);
  }
  return NULL;
}

//----------------------------------------------------------------------------
// CommandLine::add_opt
//----------------------------------------------------------------------------
void CommandLine::add_opt(char cShortName, pcsz pszLongName, pcsz pszValue, pcsz pszDesc)
{
  CommandLineOpt opt;
  opt.cShortName = cShortName;
  opt.strLongName = pszLongName;
  opt.strDesc = pszDesc;
  opt.strValue = pszValue ? pszValue : yat::String::nil;

  m_vecOptDefs.push_back(opt);
}

//----------------------------------------------------------------------------
// CommandLine::add_arg
//----------------------------------------------------------------------------
void CommandLine::add_arg(pcsz pszDesc, bool bSingle, bool bMandatory)
{
  CommandLineArg arg;
  arg.strDesc = pszDesc;
  arg.bSingle = bSingle;       // if not single Several argument can follow
  arg.bMandatory = bMandatory; // if 'true' argument is mandatory

  m_vecArgDefs.push_back(arg);
}

//----------------------------------------------------------------------------
// CommandLine::bad_option
//----------------------------------------------------------------------------
void CommandLine::bad_option(const String &strOpt) throw (Exception)
{
  std::ostringstream oss;
  oss << "Unrecognized option '" << strOpt << "'";
  std::cerr << oss << std::endl;
  show_usage(std::cerr);
  throw Exception("BAD_ARGS", oss.str(), "CommandLine::bad_option");
}

//----------------------------------------------------------------------------
// CommandLine::read
//----------------------------------------------------------------------------
bool CommandLine::read(int iArgc, char **ppszArgv) throw (Exception)
{
  // Special options that ignore all others
  add_opt('h', "help", "", "Show this usage");
  add_opt('v', "version", "", "Program version");
  
  for(int i = 1; i < iArgc; i++ )
  {
    String strArg = ppszArgv[i];
    if( strArg.start_with( "--" ) )
    {
      // Supress "--" before option name
      strArg.erase(0, 2);
      CommandLineOpt *pOpt = find_long_opt(strArg);
      if( !pOpt )
        // Bad option
        bad_option("--" + strArg);
      if( !pOpt->strValue.empty() && i < iArgc - 1 )
        // Store value
        m_dictOptValues[pOpt->strLongName] = ppszArgv[++i];
      else
        m_dictOptValues[pOpt->strLongName] = "dummy";
    }
    else if( strArg.start_with( '-') )
    {
      // Supress "-" before option name
      CommandLineOpt *pOpt = find_short_opt(strArg[1]);
      if( NULL == pOpt )
        // Bad option
        bad_option(strArg);
      if( !pOpt->strValue.empty() && i < iArgc - 1 )
        // Store value
        m_dictOptValues[pOpt->strLongName] = ppszArgv[++i];
      else
        m_dictOptValues[pOpt->strLongName] = "dummy";
    }
    else
    {
      if( m_vecArgDefs.empty() )
      {
        std::cerr << "Unexpected argument \"" << strArg << "\"" << std::endl;
        return show_usage(std::cerr);
      }

      // Simple argument
      m_vecArgs.push_back(strArg);
    }
  }

  // Check for "version" or "help" options
  if( is_option("version") )
  {
    std::cout << m_strCmdName << " V" << m_strCmdVersion << std::endl;
    return false;
  }

  else if( is_option("help") )
    return show_usage(std::cout);

  // Check arguments
  else if( m_vecArgDefs.size() > m_vecArgs.size() && m_vecArgDefs[0].bMandatory )
  {
    String error("Argument missing.");
    std::cerr << error << std::endl;
    show_usage(std::cerr);
    throw Exception("BAD_ARGS", error, "CommandLine::Read");
  }

  return true;
}

//----------------------------------------------------------------------------
// CommandLine::is_option
//----------------------------------------------------------------------------
bool CommandLine::is_option(const String &strOpt) const
{
  KeyValueMap::const_iterator it = m_dictOptValues.find(strOpt);
  if( it != m_dictOptValues.end() )
    return true;
  return false;
}

//----------------------------------------------------------------------------
// CommandLine::option_value
//----------------------------------------------------------------------------
String CommandLine::option_value(const String &strOpt) const
{
  KeyValueMap::const_iterator it = m_dictOptValues.find(strOpt);
  if( m_dictOptValues.end() == it )
    return yat::String::nil;

  return it->second;
}

//----------------------------------------------------------------------------
// CommandLine::arg_count
//----------------------------------------------------------------------------
int CommandLine::arg_count() const
{
  return m_vecArgs.size();
}

//----------------------------------------------------------------------------
// CommandLine::arg
//----------------------------------------------------------------------------
String CommandLine::arg(int i) const
{
  if( i >= 0 || i < arg_count() )
    return m_vecArgs[i];
  return yat::String::nil;
}

//----------------------------------------------------------------------------
// CommandLine::set_cmd_name_version
//----------------------------------------------------------------------------
void CommandLine::set_cmd_name_version(const String &strName, const String &strVersion)
{
  m_strCmdName = strName;
  m_strCmdVersion = strVersion;
}

//----------------------------------------------------------------------------
// CommandLine::show_usage
//----------------------------------------------------------------------------
bool CommandLine::show_usage(std::ostream &os) const
{
  os << "Usage:" << std::endl;
  os << m_strCmdName;
  if( m_vecOptDefs.size() > 0 )
    os << " [options]";
  if( m_vecArgDefs.size() > 0 )
    for( uint8 ui = 0; ui < m_vecArgDefs.size(); ui++ )
    {
      os << " " << m_vecArgDefs[ui].strDesc;
      if( !m_vecArgDefs[ui].bSingle )
        os << "...";
    }
  os << std::endl;
  if( m_vecOptDefs.size() > 0 )
  {
    os << "Options:";
    for( uint8 ui = 0; ui < m_vecOptDefs.size(); ui++ )
    {
      std::ostringstream oss;
      oss << std::endl << "-" << m_vecOptDefs[ui].cShortName << ", --" << m_vecOptDefs[ui].strLongName;
      if( !m_vecOptDefs[ui].strValue.empty() )
        oss << " <" << m_vecOptDefs[ui].strValue << ">";
      // Format Output
      os.width(40);
      os << std::left << oss.str();
      os << m_vecOptDefs[ui].strDesc;
    }
  }
  os << std::endl;
  return false;
}

//----------------------------------------------------------------------------
// CommandLine::show_usage
//----------------------------------------------------------------------------
void CommandLine::show_usage(const String &strAppInfo) const
{
  if( !strAppInfo.empty() )
    std::cout << strAppInfo << std::endl;
  show_usage(std::cout);
}

} // yat
