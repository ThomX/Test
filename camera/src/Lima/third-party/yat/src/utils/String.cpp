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
#include <yat/threading/SyncAccess.h>
#include <yat/utils/String.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// stricmp function
#ifdef YAT_LINUX
int stricmp(const char * s1, const char * s2)
{
  while( *s1 )
  {
    if( *s2 == 0 )
      return 1;

    if( toupper(*s1) < toupper(*s2) )
      return -1;
    if( toupper(*s1) > toupper(*s2) )
      return 1;
    s1++;
    s2++;
}

  if( *s2 == 0 )
    return 0;
  return -1;
}

int strnicmp(const char * s1, const char * s2, int maxlen)
{
  int i = 0;
  while( i<maxlen )
  {
    if( *s2 == 0 || *s1 == 0 )
      return 1;

    if( toupper(*s1) < toupper(*s2) )
      return -1;
    if( toupper(*s1) > toupper(*s2) )
      return 1;
    s1++;
    s2++;
    i++;
  }
  return 0;
}
#endif

namespace yat
{

// Simple scratch buffer
const int g_iScratchLen = 262144;
char g_acScratchBuf[g_iScratchLen];

//=============================================================================
//
// String
//
//=============================================================================
const String String::nil = "";
//---------------------------------------------------------------------------
// Build a string using C-like format
//---------------------------------------------------------------------------
String String::str_format(pcsz pszFormat, ...)
{
  LOCK(&g_acScratchBuf)
  va_list argptr;
  va_start(argptr, pszFormat);
  VSNPRINTF(g_acScratchBuf, g_iScratchLen, pszFormat, argptr);
  va_end(argptr);

  return String(g_acScratchBuf);
}

//---------------------------------------------------------------------------
// String::extract_token
//---------------------------------------------------------------------------
String::ExtractTokenRes String::extract_token(char c, String *pstrToken)
{
  // Cannot extract a substring a put it in the same string !
  if( this == pstrToken )
    return EMPTY_STRING;

  int iSrcLength = length();

  if( 0 == iSrcLength )
  {
    // Nothing else
    pstrToken->erase();
    return EMPTY_STRING;
  }

  // Search for separator
  int iPos = find_first_of(c);
  if( iPos < 0 )
  {
    // Not found
    *pstrToken = *this;
    *this = nil;
    return SEP_NOT_FOUND;
  }

  // Separator found
  *pstrToken = substr(0, iPos);
  erase(0, iPos+1);
  return SEP_FOUND;
}

//---------------------------------------------------------------------------
// String::split
//---------------------------------------------------------------------------
void String::split(char c, String *pstrLeft, String *pstrRight, bool bPreserve)
{
  if( bPreserve )
  {
    String strSaved(*this);
    extract_token_right(c, pstrRight);
    (*pstrLeft) = *this;
    *this = strSaved;
  }
  else
  {
    extract_token_right(c, pstrRight);
    (*pstrLeft) = *this;
  }
}

//---------------------------------------------------------------------------
// String::extract_token_right
//---------------------------------------------------------------------------
String::ExtractTokenRes String::extract_token_right(char c, String *pstrToken)
{
  // Cannot extract a substring a put it in the same string !
  if( this == pstrToken )
    return EMPTY_STRING;

  int iSrcLength = length();

  if( 0 == iSrcLength )
  {
    // Nothing else
    pstrToken->erase();
    return EMPTY_STRING;
  }

  // Search for separator
  int iPos = find_last_of(c);
  if( iPos < 0 )
  {
    // Not found
    *pstrToken = *this;
    *this = nil;
    return SEP_NOT_FOUND;
  }

  // Separator found
  *pstrToken = substr(iPos+1);
  erase(iPos);
  return SEP_FOUND;
}

//---------------------------------------------------------------------------
// String::extract_token
//---------------------------------------------------------------------------
String::ExtractTokenRes String::extract_token(char cLeft, char cRight, String *pstrToken)
{
  // Cannot extract a substring a put it in the same string !
  if( this == pstrToken )
    return EMPTY_STRING;

  int iSrcLength = length();

  if( 0 == iSrcLength )
  {
    // Nothing else
    pstrToken->erase();
    return EMPTY_STRING;
  }

  // Search for enclosing characters
  int iLeftPos = find(cLeft);
  int iRightPos = find(cRight, iLeftPos + 1);
  if( iLeftPos < 0 || iRightPos < 0 || iRightPos < iLeftPos )
  {
    // Not found
    *pstrToken = nil;
    return SEP_NOT_FOUND;
  }

  // Enclosing characters found
  *pstrToken = substr(iLeftPos + 1, iRightPos - iLeftPos - 1);
  erase(0, iRightPos + 1);
  return SEP_FOUND;
}

//---------------------------------------------------------------------------
// String::extract_token_right
//---------------------------------------------------------------------------
String::ExtractTokenRes String::extract_token_right(char cLeft, char cRight, String *pstrToken)
{
  // Cannot extract a substring a put it in the same string !
  if( this == pstrToken )
    return EMPTY_STRING;

  int iSrcLength = length();

  if( 0 == iSrcLength )
  {
    // Nothing else
    pstrToken->erase();
    return EMPTY_STRING;
  }

  // Search for enclosing characters
  int iRightPos = rfind(cRight);
  int iLeftPos = iRightPos > 0 ? (int)rfind(cLeft, iRightPos - 1) : -1;
  if( iLeftPos < 0 || iRightPos < 0 || iRightPos < iLeftPos )
  {
    // Not found
    *pstrToken = nil;
    return SEP_NOT_FOUND;
  }

  // Enclosing characters found
  *pstrToken = substr(iLeftPos+1, iRightPos - iLeftPos - 1);
  erase(iLeftPos);
  return SEP_FOUND;
}

//---------------------------------------------------------------------------
// String::remove_enclosure
//---------------------------------------------------------------------------
bool String::remove_enclosure(psz pszLeft, psz pszRight)
{
  // pcszLeft & pcszRight must have the same length
  if( strlen(pszLeft) != strlen(pszRight) )
    return false;
  
  for( uint32 ui = 0; ui < strlen(pszLeft); ui++ )
  {
    std::string strMask;
    strMask += pszLeft[ui];
    strMask += '*';
    strMask += pszRight[ui];
    if( match(strMask) )
    {
      *this = substr(strlen(pszLeft), size() - (strlen(pszLeft) + strlen(pszRight)));
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------
// String::remove_enclosure
//---------------------------------------------------------------------------
bool String::remove_enclosure(char cLeft, char cRight)
{
  std::string strMask;
  strMask += cLeft;
  strMask += '*';
  strMask += cRight;
  if( match(strMask) )
  {
    *this = substr(1, size() - 2);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
// String::is_equal
//---------------------------------------------------------------------------
bool String::is_equal(const String &str) const
{
  return (*this == str);
}

//---------------------------------------------------------------------------
// String::is_equal_no_case
//---------------------------------------------------------------------------
bool String::is_equal_no_case(const String &str) const
{
  return (!stricmp(c_str(), str.c_str()));
}

//---------------------------------------------------------------------------
// String::start_with
//---------------------------------------------------------------------------
bool String::start_with(char c) const
{
  if( size() == 0 )
    return false;

  if( c == (*this)[0] )
    return true;

  return false;
}

//---------------------------------------------------------------------------
// String::start_with
//---------------------------------------------------------------------------
bool String::start_with(pcsz pcszStart, bool bNoCase) const
{
  if( size() < strlen(pcszStart) )
    return false;

  if( bNoCase )
  {
    return (!strnicmp(c_str(), pcszStart, strlen(pcszStart)));
  }
  else
  {
    return (!strncmp(c_str(), pcszStart, strlen(pcszStart)));
  }
}

//---------------------------------------------------------------------------
// String::end_with
//---------------------------------------------------------------------------
bool String::end_with(pcsz pcszEnd, bool bNoCase) const
{
  if( size() < strlen(pcszEnd) )
    return false;

  if( bNoCase )
  {
    return (!strnicmp(c_str()+strlen(c_str())-strlen(pcszEnd), pcszEnd, strlen(pcszEnd)));
  }
  else
  {
    return (!strncmp(c_str()+strlen(c_str())-strlen(pcszEnd), pcszEnd, strlen(pcszEnd)));
  }
}

//---------------------------------------------------------------------------
// String::end_with
//---------------------------------------------------------------------------
bool String::end_with(char c) const
{
  if( size() == 0 )
    return false;

  if( c == (*this)[size()-1] )
    return true;

  return false;
}

//---------------------------------------------------------------------------
// Internal utility function
// Look for occurence for a string in another
// Take care of '?' that match any character
//---------------------------------------------------------------------------
static pcsz find_sub_str_with_joker(pcsz pszSrc, pcsz pMask, uint32 uiLenMask)
{
  if (strlen(pszSrc) < uiLenMask)
    return NULL; // No hope

  // while mask len < string len 
  while( *(pszSrc + uiLenMask - 1) ) 
  {
    uint32 uiOffSrc = 0; // starting offset in mask and sub-string
    
    // Tant qu'on n'est pas au bout du masque
    while (uiOffSrc < uiLenMask)
    {
      char cMask = pMask[uiOffSrc];
      
      if (cMask != '?') // In case of '?' it always match
      {
        if (pszSrc[uiOffSrc] != cMask)
          break;
      }
  
      // Next char
      uiOffSrc++;
    }

    // String matched !
    if (uiOffSrc == uiLenMask)
      return pszSrc + uiLenMask;

    // Next sub-string
    pszSrc++;
  }

  // Not found
  return NULL;
}

//---------------------------------------------------------------------------
// String::match
//---------------------------------------------------------------------------
bool String::match(const String &strMask) const
{
  return match(PSZ(strMask));
}

//---------------------------------------------------------------------------
// String::match
//---------------------------------------------------------------------------
bool String::match(pcsz pszMask) const
{
  pcsz pszTxt = c_str();
  while (*pszMask)
  {
    switch (*pszMask)
    {

      case '\\':
        // escape next special mask char (e.g. '?' or '*')
        pszMask++;
        if( *pszMask )
        {
          if( *(pszMask++) != *(pszTxt++) )
            return false;
        }
        break;

      case '?': // joker at one position
        if (!*pszTxt)
          return true; // no match

        pszTxt++;
        pszMask++;
        break;

      case '*': // joker on one or more positions
        {
          // Pass through others useless joker characters
          while (*pszMask == '*' || *pszMask == '?')
            pszMask++;

          if (!*pszMask)
            return true; // Fin

          // end of mask
          uint32 uiLenMask;
          const char *pEndMask = strchr(pszMask, '*');

          if (pEndMask)
            // other jokers characters => look for bloc between the two jokers in source string
            uiLenMask = pEndMask - pszMask;
          else
            // string must be end with mask
            return (NULL != find_sub_str_with_joker(pszTxt + strlen(pszTxt)-strlen(pszMask), pszMask, strlen(pszMask)))?
                           true : false;

          // Search first uiLenMask characters from mask in text
          pcsz pEnd = find_sub_str_with_joker(pszTxt, pszMask, uiLenMask);

          if (!pEnd)
            // Mask not found
            return false;

          pszTxt = pEnd;
          pszMask += uiLenMask;
        }
        break;

      default:
        if( *(pszMask++) != *(pszTxt++) )
          return false;
        break;
    }
  }

  if( *pszTxt )
    // End of string not reached
    return false;

  return true;
}

//---------------------------------------------------------------------------
// String::trim
//---------------------------------------------------------------------------
void String::trim()
{
  if( size() > 0 )
  {
    int iFirstNoWhite = 0;
    int iLastNoWhite = size()-1;

    // Search for first non-white character
    for( ; iFirstNoWhite <= iLastNoWhite; iFirstNoWhite++ )
    {
      if( !isspace((*this)[iFirstNoWhite]) )
        break;
    }

    // Search for last non-white character
    for( ; iLastNoWhite > iFirstNoWhite; iLastNoWhite-- )
    {
      if( !isspace((*this)[iLastNoWhite]) )
        break;
    }

    // Extract sub-string
    (*this) = substr(iFirstNoWhite, iLastNoWhite - iFirstNoWhite + 1);
  }
}

//---------------------------------------------------------------------------
// String::printf
//---------------------------------------------------------------------------
int String::printf(pcsz pszFormat, ...)
{
  LOCK(&g_acScratchBuf)
  va_list argptr;
  va_start(argptr, pszFormat);
  VSNPRINTF(g_acScratchBuf, g_iScratchLen, pszFormat, argptr);
  va_end(argptr);

  *this = g_acScratchBuf;
  return size();
}

//---------------------------------------------------------------------------
// String::split
//---------------------------------------------------------------------------
void String::split(char c, std::vector<String> *pvecstr, bool bClearVector)
{
  // Clear vector
  if( bClearVector )
    pvecstr->clear();
  String strToken;
  while( !empty() )
  {
    if( 0 != extract_token(c, &strToken) )
      pvecstr->push_back(strToken);
  }
}

//---------------------------------------------------------------------------
// String::split (const version)
//---------------------------------------------------------------------------
void String::split(char c, std::vector<String> *pvecstr, bool bClearVector) const
{
  // Clear vector
  if( bClearVector )
    pvecstr->clear();
  String strToken;
  String strTmp(*this);
  while( !strTmp.empty() )
  {
    if( 0 != strTmp.extract_token(c, &strToken) )
      pvecstr->push_back(strToken);
  }
}

//---------------------------------------------------------------------------
// String::join
//---------------------------------------------------------------------------
void String::join(const std::vector<String> &vecStr, char cSep)
{
  erase();
  for( uint32 ui=0; ui < vecStr.size(); ui++ )
  {
    if( 0 < ui )
      *this += cSep;
    *this += vecStr[ui];
  }
}

//---------------------------------------------------------------------------
// String::remove_item
//---------------------------------------------------------------------------
bool String::remove_item(const String &strItem, char cSep)
{
  String::size_type uiPos = find(strItem);
  if( uiPos == std::string::npos )
    return false;

  if( *this == strItem )
  {
    erase();
    return true;
  }

  std::vector<String> vecstr;
  split(cSep, &vecstr);
  for( std::vector<String>::iterator it = vecstr.begin(); it != vecstr.end(); it++ )
  {
    if( *it == strItem )
    {
      vecstr.erase(it);
      join(vecstr, cSep);
      return true;
    }
  }
  join(vecstr, cSep);
  return false;
}

//---------------------------------------------------------------------------
// String::lower
//---------------------------------------------------------------------------
void String::to_lower()
{
  for(uint32 ui=0; ui < size(); ui++)
    std::string::replace(ui, 1, 1, tolower(c_str()[ui]));
}

//---------------------------------------------------------------------------
// String::upper
//---------------------------------------------------------------------------
void String::to_upper()
{
  for(uint32 ui=0; ui < size(); ui++)
    std::string::replace(ui, 1, 1, toupper(c_str()[ui]));
}

//---------------------------------------------------------------------------
// String::replace
//---------------------------------------------------------------------------
void String::replace(pcsz pszSrc, pcsz pszDst)
{
  pcsz pBeg = NULL, pFind = NULL;

  int iLenSrc = strlen(pszSrc);
  if( iLenSrc == 0 )
    // Avoid infinite loop
    return;

  int iLenDst = strlen(pszDst);
  int iOffset = 0;
  for(;;)
  {
    pBeg = c_str() + iOffset;
    pFind = strstr(pBeg, pszSrc);

    if( !pFind )
      return;

    // Do replace
    std::string::replace(pFind - pBeg + iOffset, iLenSrc, pszDst);
    // replace again after replacement
    iOffset += pFind - pBeg + iLenDst;
  }
}

//---------------------------------------------------------------------------
// String::replace
//---------------------------------------------------------------------------
void String::replace(char cSrc, char cDst)
{
  if (!strchr(c_str(), cSrc))
    return; // Nothing to do

  for( uint32 uiPos = 0; uiPos < size(); uiPos++ )
    if( cSrc == (*this)[uiPos] )
      std::string::replace(uiPos, 1, 1, cDst);
}

//---------------------------------------------------------------------------
// String::hash
//---------------------------------------------------------------------------
uint32 String::hash() const
{
  // Very basic implementation !
  int64 modulo = (int64(2) << 31) - 1;
  int64 hash64 = 0;
  uint32 length = size();
  for( uint32 i = 0; i < length; i++ )
    hash64 = (31 * hash64 + (*this)[i]) % modulo;

  return (uint32)(hash64); 
}

//---------------------------------------------------------------------------
// String::hash64
//---------------------------------------------------------------------------
uint64 String::hash64() const
{
  // Implementation of the FNV-1a algorithm (http://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function)
  uint64 hash64 = uint64(14695981039346656037ULL);
  uint32 length = size();
  for( uint32 i = 0; i < length; i++ )
    hash64 = (hash64 ^ (*this)[i]) * uint64(1099511628211ULL);

  return hash64; 
}


} // namespace