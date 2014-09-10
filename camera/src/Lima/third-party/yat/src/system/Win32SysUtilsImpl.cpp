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
#include <yat/threading/SyncAccess.h>
#include <shellapi.h>

namespace yat
{

//----------------------------------------------------------------------------
// SysUtils::GetEnv
//----------------------------------------------------------------------------
bool SysUtils::get_env(const String &strVar, String *pstrValue, const char *pszDef)
{
  LOCK(&g_acScratchBuf);
  if ( ::GetEnvironmentVariable(PSZ(strVar), g_acScratchBuf, g_iScratchLen) == 0 )
  { // if the variable  is undefined, use default value
    if ( pszDef )
      *pstrValue = pszDef;
    return false;
  }

  *pstrValue = g_acScratchBuf;
  return true;
}

//----------------------------------------------------------------------------
// SysUtils::exec
//----------------------------------------------------------------------------
bool SysUtils::exec(const char* pszCmdLine, const char *pszDefDir, int bBackground, bool bThrow, int *puiReturnCode)
{
  int rc = 0;

  // First of all we want to known if the command line refer to a executable or a document
  // Because of a different behavior of the GetMessage() function
  bool bIsExecutable;
  String strCommandLine = pszCmdLine;
  String strCmd;
  strCommandLine.trim();

  if ( strCommandLine.start_with('"') )
  { // the executable name of path may contains spaces. Normaly the method caller have
    // demilited the command with quotes.
    int iQuotIndex = strCmd.find('"');
    if( iQuotIndex == -1 )
    {
      if ( bThrow )
        throw Exception("SYSTEM_ERROR", "Missing quotes", "SysUtils::exec_as");
    }

    strCmd = strCommandLine.substr(1, iQuotIndex);
  }
  else
    strCommandLine.extract_token(' ', &strCmd);

  // strCmd is the first argument, e.g the name of a executable or a document
  // Extract the extention
  String strExt;
  String strCmdDummy = strCmd;
  rc = strCmdDummy.extract_token_right('.', &strExt );
  if( rc != String::SEP_FOUND )
  {
    // No extension => we consider it's a executable
    strExt = String::nil;
    bIsExecutable = true;
  }
  else
  {
    // Looking for excutable extentions
    if(   stricmp( PSZ(strExt), "exe" ) == 0
       || stricmp( PSZ(strExt), "com" ) == 0
       || stricmp( PSZ(strExt), "bat" ) == 0
       || stricmp( PSZ(strExt), "pif" ) == 0
       || stricmp( PSZ(strExt), "cmd" ) == 0
      )
    {
      // Rejection of the URLs
      if (strstr(PSZ(strCmd), "://") == NULL)
        bIsExecutable = true;
      else
        bIsExecutable = false;
    }
    else
      bIsExecutable = false;
  }

  bool bIsError = false;

  // For a executable we use CreateProcess
  PROCESS_INFORMATION aProcessInformation;
  memset(&aProcessInformation, 0, sizeof(PROCESS_INFORMATION));
  if( bIsExecutable )
  {
    STARTUPINFO         aStartupInfo;
    memset(&aStartupInfo, 0, sizeof(aStartupInfo));
    aStartupInfo.cb = sizeof(aStartupInfo);
    rc = 0;
    if( !::CreateProcess(NULL, (char*)pszCmdLine, NULL, NULL,
                         FALSE, CREATE_NO_WINDOW, NULL,
                         pszDefDir,
                         &aStartupInfo, &aProcessInformation) )
    {
      rc = ::GetLastError();
      bIsError = true;
    }
  }
  else
  {
    // This not a executable, we must use ShellExecute
    rc = (int)::ShellExecute( HWND_DESKTOP, NULL,
                              PSZ(strCmd),
                              PSZ(strCommandLine),
                              pszDefDir,
                              SW_SHOW );
    if( rc >= 0 && rc < 32 )
      bIsError = true;
  }

  if( !bBackground && bIsExecutable)
  {
    // Waiting for the process is done
    DWORD uiReturnCode;
    ::WaitForSingleObject( aProcessInformation.hProcess, INFINITE);
    ::GetExitCodeProcess(aProcessInformation.hProcess, &uiReturnCode);
    if( puiReturnCode )
      *puiReturnCode = static_cast<int>(uiReturnCode);
    return true;
  }

  if ( bIsExecutable )
  { 
    // Free system handles
    ::CloseHandle(aProcessInformation.hProcess);
    ::CloseHandle(aProcessInformation.hThread);
  }
  
  if( bIsError && bThrow )
    throw Exception("ERR_FILE", "Error while executing command", "Exec");

  return true;
}

//----------------------------------------------------------------------------
// SysUtils::exec_as
//----------------------------------------------------------------------------
bool SysUtils::exec_as(const char* pszCmdLine, const char *pszDefDir, int bBackground, bool bThrow, int *puiReturnCode, uid_t uid, gid_t gid)
{
  // On Win32 no identity change !
  return exec(pszCmdLine, pszDefDir, bBackground, bThrow, puiReturnCode);
}


} // namespace
