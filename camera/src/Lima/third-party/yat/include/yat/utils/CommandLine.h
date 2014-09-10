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

#ifndef _YAT_COMMANDLINE_H_
#define _YAT_COMMANDLINE_H_

#include <map>
#include <iostream>
#include <sstream>

#include <yat/CommonHeader.h>
#include <yat/utils/String.h>
#include <yat/Exception.h>

namespace yat 
{


// ============================================================================
//! \class CommandLine 
//! \brief A simple command line parser.
//!
//! This class provides a parser for a command line that contains options (i.e. "-opt val")
//! and simple arguments (i.e. "arg1 arg2 ..."). The command line format is like:
//! \verbatim  $my_app -a -b \<value_of_b\> -c \<value_of_c\> arg1 arg2...  \endverbatim
//! 
// ============================================================================
class YAT_DECL CommandLine
{
public:
  //=============================================================================
  //! \brief Command line option.
  //=============================================================================
  struct CommandLineOpt
  {
    char   cShortName;
    String strLongName;
    bool   bMandatory;
    String strDesc;
    String strValue;
  };

  //! \brief Vector of command line options.
  typedef std::vector<CommandLineOpt> vecOpts;

  //=============================================================================
  //! \brief Command line argument.
  //=============================================================================
  struct CommandLineArg
  {
    String strDesc;
    bool   bMandatory;
    bool   bSingle;
  };

  //! \brief Vector of command line arguments.
  typedef std::vector<CommandLineArg> vecArgs;

  //! \brief Sets command name and version.
  //!
  //! These names will be displayed with the *show_usage()* function.
  //! \param strName Command's name.
  //! \param strVersion Command's version.
  void set_cmd_name_version(const String &strName, const String &strVersion);

  //! \brief Defines an option for the command line.
  //!
  //! \param cShortName Option's short name.
  //! \param pszLongName Option's long name.
  //! \param pszValue Option's value.
  //! \param pszDesc Option's description.
  void add_opt(char cShortName, pcsz pszLongName, pcsz pszValue, pcsz pszDesc);

  //! \brief Defines a simple argument for the command line.
  //! 
  //! \param pszDesc Argument's description.
  //! \param bSingle If set to false, several arguments can follow.
  //! \param bMandatory If set to true, argument is mandatory.
  void add_arg(pcsz pszDesc, bool bSingle=true, bool bMandatory=true);

  //! \brief Initializes the command line with arguments (options and simple arguments).
  //!
  //! Returns true if arguments are sufficient to launch application, false otherwise.
  //! \param iArgc Number of arguments.
  //! \param ppszArgv List of arguments.
  //! \exception BAD_ARGS Thrown if arguments are not valid.
  bool read(int iArgc, char **ppszArgv) throw (Exception);

  //! \brief Shows command's usage.
  //!
  //! \param[out] strAppInfo Additional information about the command
  //! or application.
  void show_usage(const String &strAppInfo) const;

  //! \brief Checks if option is defined.
  //!
  //! Returns true if the specified option is defined for the command, false otherwise.
  //! \param[in] strArg The option's name to retrieve.
  bool is_option(const String &strArg) const;

  //! \brief Gets options's value.
  //!
  //! Returns the option's value or nil string if not found.
  //! \param[in] strOpt The option's name to retrieve.
  String option_value(const String &strOpt) const;
  
  //! \brief Gets the number of initialized arguments.
  //! 
  //! Returns the number of simple arguments initialized with the *read()* function.
  int arg_count() const;

  //! \brief Gets a simple argument by its rank.
  //!
  //! Returns the \<i\>th simple argument's value initialized with the *read()* function.
  //! \param i Rank of the simple argument to retrieve.
  String arg(int i) const;

private:

  typedef std::map<String, String> KeyValueMap;
  
  //- Current options values
  KeyValueMap m_dictOptValues; 
  
  //- Simples arguments (not options)
  std::vector<String> m_vecArgs;

  //- Valid options list
  vecOpts     m_vecOptDefs;

  //- Valid arguments list
  vecArgs     m_vecArgDefs;

  //- Argument dictionary
  KeyValueMap m_dictOpts;

  //- Command name
  String      m_strCmdName;

  //- Version
  String      m_strCmdVersion;

  //- Find option def from short form
  CommandLineOpt *find_short_opt(const char cOpt);

  //- Find option definition from long form
  CommandLineOpt *find_long_opt(const String &strOpt);

  //- Display "bad option" error message
  //-
  //- return always false
  void bad_option(const String &strOpt) throw (Exception);

  //- Show usage
  //-
  //- return false
  bool show_usage(std::ostream &os = std::cout) const;
};

} // yat

#endif
