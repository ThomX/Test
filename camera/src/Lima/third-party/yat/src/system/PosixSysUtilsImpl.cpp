//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2012  The Tango Community
//
// Part of the code comes from the ACE Framework (asm bytes swaping code)
// see http://www.cs.wustl.edu/~schmidt/ACE.html for more about ACE
//
// The thread native implementation has been initially inspired by omniThread
// - the threading support library that comes with omniORB.
// see http://omniorb.sourceforge.net/ for more about omniORB.
//
// Contributors form the TANGO community:
// Ramon Sune (ALBA) for the Signal class
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
#include <yat/system/SysUtils.h>
#include <stdlib.h>
#include <sys/wait.h>

namespace yat
{

//----------------------------------------------------------------------------
// SysUtils::get_env
//----------------------------------------------------------------------------
bool SysUtils::get_env(const String &strVar, String *pstrValue, const char *pszDef)
{
  char *pszBuf = ::getenv(PSZ(strVar));
  if ( pszBuf == NULL )
  { // if the variable  is undefined, use default value
    if ( pszDef )
      *pstrValue = pszDef;
    return false;
  }

  *pstrValue = pszBuf;
  return true;
}

//----------------------------------------------------------------------------
// SysUtils::exec
//----------------------------------------------------------------------------
bool SysUtils::exec(const char* pszCmdLine, const char *, int bBackground, bool, int *puiReturnCode)
{
  String sCmd = pszCmdLine;
  if( bBackground )
    // Background task
    sCmd += "&";

  // Execute shell command then exit
  *puiReturnCode = ::system(PSZ(sCmd));

  return true;
}

//----------------------------------------------------------------------------
// SysUtils::exec_as
//----------------------------------------------------------------------------
bool SysUtils::exec_as(const char* pszCmdLine, const char *, int bBackground, bool bThrow, int *puiReturnCode, uid_t uid, gid_t gid)
{
  int rc = 0;

  String sCmd = pszCmdLine;
  if( bBackground )
    // Background task
    sCmd += "&";

  // fork a new process
  pid_t pid = ::fork();
  if( 0 == pid )
  {
    // Child process
    if( gid > 0 )
    {
      // Set new gid
      rc = ::setgid(gid);
      if( rc )
        ::exit(-1);
    }
    if( uid > 0 )
    {
      // Set new uid
      rc = ::setuid(uid);
      if( rc )
        ::exit(-1);
    }
    // Execute shell command then exit
    ::exit(::system(PSZ(sCmd)));
  }
  else if( pid < 0 && bThrow )
    throw Exception("SYSTEM_ERROR", "Error cannot execute shell command", "SysUtils::exec_as");
  else if( pid < 0 )
    return false;
    
  // Parent process, let's wait for the child to finish
  ::waitpid(pid, puiReturnCode, WEXITSTATUS(*puiReturnCode));
  return true;
}

} // namespace
