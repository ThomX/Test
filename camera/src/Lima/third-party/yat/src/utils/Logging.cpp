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
// see AUTHORS file 
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
#include <yat/utils/Logging.h>
#include <yat/time/Time.h>
#include <yat/threading/SyncAccess.h>
#include <iostream>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

namespace yat
{
//=============================================================================
//
// LogManager
//
//=============================================================================
LogManager *LogManager::ms_pTheInstance = NULL;

//----------------------------------------------------------------------------
// LogManager::LogManager
//----------------------------------------------------------------------------
LogManager::LogManager() : m_iMinLevel(LOG_INFO)
{
}

//----------------------------------------------------------------------------
// LogManager::Instance
//----------------------------------------------------------------------------
LogManager *LogManager::Instance()
{
  if( NULL == ms_pTheInstance )
    ms_pTheInstance = new LogManager;

  return ms_pTheInstance;
}

//----------------------------------------------------------------------------
// LogManager::push_log_target
//----------------------------------------------------------------------------
void LogManager::push_log_target(ILogTarget *pLogTarget)
{
  Instance()->m_stkCatchLogTarget.push(pLogTarget);
}

//----------------------------------------------------------------------------
// LogManager::pop_log_target
//----------------------------------------------------------------------------
void LogManager::pop_log_target()
{
  // Get a reference to the singleton object
  LogManager &o = *Instance();

  if( !o.m_stkCatchLogTarget.empty() )
    o.m_stkCatchLogTarget.pop();
}

//----------------------------------------------------------------------------
// LogManager::current_log_target
//----------------------------------------------------------------------------
ILogTarget *LogManager::current_log_target()
{
  // Get a reference to the singleton object
  LogManager &o = *Instance();

  if( !o.m_stkCatchLogTarget.empty() )
    return o.m_stkCatchLogTarget.top();

  return &o.m_defLogHandler;
}

//----------------------------------------------------------------------------
// LogManager::init
//----------------------------------------------------------------------------
void LogManager::init(int iMinLevel, const String &_strFilter)
{
  // Get a reference to the singleton object
  LogManager &o = *Instance();

  o.m_iMinLevel = iMinLevel;

  String strFilter = _strFilter, strType;
  while( strFilter.size() > 0 )
  {
    strFilter.extract_token('|', &strType);
    if( strType.size() > 0 )
      o.m_setTypes.insert(strType);
  }
}

//----------------------------------------------------------------------------
// LogManager::log
//----------------------------------------------------------------------------
void LogManager::log(ELogLevel eLevel, pcsz pszType, const String &strMsg)
{
  // Get a reference to the singleton
  LogManager &o = *Instance();

  // Minimum level message to display (except for LOG_RESULT type messages)
  if( int(eLevel) < o.m_iMinLevel && eLevel != LOG_RESULT )
    // Do nothing
    return;

  // verbose messages filtering
  if( o.m_setTypes.size() > 0 &&
      o.m_setTypes.find(String(pszType)) == o.m_setTypes.end() && LOG_VERBOSE == eLevel )
    // Type not found in filter set
    return;
  
  // Notify log tarteg
  if( current_log_target() )
    current_log_target()->log(eLevel, pszType, strMsg);
}

//=============================================================================
//
// DefaultLogHandler
//
//=============================================================================
//----------------------------------------------------------------------------
// DefaultLogHandler::log
//----------------------------------------------------------------------------
void DefaultLogHandler::log(ELogLevel eLevel, pcsz pszType, const String &strMsg)
{
  // Formatting message
  CurrentTime dtCur;
  String strLogDate = String::str_format("%4d-%02d-%02d,%02d:%02d:%02.3f", 
                                        dtCur.year(), dtCur.month(), dtCur.day(),
                                        dtCur.hour(), dtCur.minute(), dtCur.second());
  
  String strLevel;
  switch( eLevel )
  {
    case LOG_EMERGENCY:
      strLevel = "EMERGENCY";
      break;
    case LOG_ALERT:
      strLevel = "ALERT";
      break;
    case LOG_CRITICAL:
      strLevel = "CRITICAL";
      break;
    case LOG_ERROR:
      strLevel = "ERROR";
      break;
    case LOG_WARNING:
      strLevel = "WARNING";
      break;
    case LOG_NOTICE:
      strLevel = "NOTICE";
      break;
    case LOG_INFO:
      strLevel = "INFO";
      break;
    case LOG_VERBOSE:
      strLevel = "DEBUG";
      break;
    case LOG_RESULT:
      strLevel = "RESULT";
      break;
    case LOG_LEVEL_COUNT:
    default:
      break;
  }
  std::clog << strLogDate << ' ' << strLevel << " [" << pszType << "]:" << strMsg << '\n';
};

//=============================================================================
//
// LogCatcher
//
//=============================================================================
//----------------------------------------------------------------------------
// LogCatcher::LogCatcher
//----------------------------------------------------------------------------
LogCatcher::LogCatcher(ILogTarget *pLogTarget)
{
  LogManager::push_log_target(pLogTarget);
};

//----------------------------------------------------------------------------
// LogCatcher::~LogCatcher
//----------------------------------------------------------------------------
LogCatcher::~LogCatcher()
{
  LogManager::pop_log_target();
};

//=============================================================================
//
// LogForward
//
//=============================================================================
//----------------------------------------------------------------------------
// LogForward::LogForward
//----------------------------------------------------------------------------
LogForward::LogForward(pfn_log_fwd pfn_log_fwd)
{
  m_pfn_log_fwd = pfn_log_fwd;
}

//----------------------------------------------------------------------------
// LogForward::Log
//----------------------------------------------------------------------------
void LogForward::log(ELogLevel eLevel, pcsz pszType, const String &strMsg)
{
  if( m_pfn_log_fwd )
    m_pfn_log_fwd(eLevel, pszType, PSZ(strMsg));
}

//=============================================================================
// Macro for Log functions
//=============================================================================
#define LOG_MSG(level)                                          \
  LOCK(&g_acScratchBuf)                                         \
  va_list argptr;                                               \
  va_start(argptr, pszFormat);                                  \
  VSNPRINTF(g_acScratchBuf, g_iScratchLen, pszFormat, argptr);  \
  va_end(argptr);                                               \
  String strMsg = g_acScratchBuf;                               \
  LogManager::log(level, pszType, strMsg);

// Log functions
void log_result(pcsz pszType, pcsz pszFormat, ...)    { LOG_MSG(LOG_RESULT) }
void log_verbose(pcsz pszType, pcsz pszFormat, ...)   { LOG_MSG(LOG_VERBOSE) }
void log_info(pcsz pszType, pcsz pszFormat, ...)      { LOG_MSG(LOG_INFO) }
void log_notice(pcsz pszType, pcsz pszFormat, ...)    { LOG_MSG(LOG_NOTICE) }
void log_warning(pcsz pszType, pcsz pszFormat, ...)   { LOG_MSG(LOG_WARNING) }
void log_error(pcsz pszType, pcsz pszFormat, ...)     { LOG_MSG(LOG_ERROR) }
void log_critical(pcsz pszType, pcsz pszFormat, ...)  { LOG_MSG(LOG_CRITICAL) }
void log_alert(pcsz pszType, pcsz pszFormat, ...)     { LOG_MSG(LOG_ALERT) }
void log_emergency(pcsz pszType, pcsz pszFormat, ...) { LOG_MSG(LOG_EMERGENCY) }

} // namespace