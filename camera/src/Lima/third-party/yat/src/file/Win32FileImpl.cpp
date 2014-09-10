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
#include <yat/time/Time.h>
#include <yat/file/FileName.h>
#include <yat/threading/SyncAccess.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace yat
{

///===========================================================================
/// FileName
///===========================================================================

//----------------------------------------------------------------------------
// FileName::access_from_string
//----------------------------------------------------------------------------
mode_t FileName::access_from_string(const String &strAccess)
{
  mode_t mode = 0;
  return mode;
}

//-------------------------------------------------------------------
// FileName::path_exist()
//-------------------------------------------------------------------
bool FileName::path_exist() const
{
  pcsz pszPath = PSZ(full_name());
  if( strchr(pszPath, '*') || strchr(pszPath, '?') )
    // there are wildcard. this is not a valid path
    return false;
 
  uint32 uiLen = strlen(pszPath) ;
  if (uiLen == 0)
     return false;
 
  WIN32_FIND_DATA find;
  HANDLE          h;
  if( pszPath[uiLen-1] == '\\' )
  {
    if( uiLen >= 2 && pszPath[uiLen-2] == ':' )
    {
      // Path is a disque drive name
      uint32 uiRc = ::GetDriveType( pszPath );
      if( uiRc == 0 || uiRc == 1 )
        return false;
      return true;
    }
    // Path ends with '\' => remove '\'
    String strPath = pszPath;
    strPath = strPath.substr(0, strPath.size()-1);
    h = FindFirstFile( PSZ(strPath), &find );
  }
  else
    h = FindFirstFile( pszPath, &find );
 
  if( h == INVALID_HANDLE_VALUE )
    return false;
  FindClose( h );
  return (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

//-------------------------------------------------------------------
// FileName::file_exist
//-------------------------------------------------------------------
bool FileName::file_exist() const
{
  pcsz pcszfull_name = PSZ(full_name());
  WIN32_FIND_DATA find;
  HANDLE h = FindFirstFile(pcszfull_name, &find);
  if( h == INVALID_HANDLE_VALUE )
    return false;
  FindClose( h );
  return (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

//-------------------------------------------------------------------
// FileName::file_access
//-------------------------------------------------------------------
bool FileName::file_access() const
{
  return this->file_exist();
}

//----------------------------------------------------------------------------
// FileName::set_full_name
//----------------------------------------------------------------------------
void FileName::set_full_name(pcsz pszFileName)
{
  if( !pszFileName || !pszFileName[0] )
  {
    m_strFile = String::nil;
    return;
  }

  String strFileName = pszFileName;

  // Convert separators
  convert_separators(&strFileName);

  m_strFile = strFileName;
 
  // Warning: strFile may be empty
  if( m_strFile.size() > 0 && 
      ( m_strFile.start_with("\\\\") ||    // UNC '\\'
        m_strFile[1u] == SEP_DSK    ) )   // Drive letter: 'X:'
  { 
    // Absolute name

  }
  else
  {
    String strDir;
    {
      LOCK(&g_acScratchBuf)
      _getcwd(g_acScratchBuf, _MAX_PATH);
      strDir = g_acScratchBuf;
    }

    // Convert path into full name in windows format.
    // 3 cases :
    // - path begin with /cygdrive/<letter>/ => absolute path name
    //   We replace this part by the drive letter
    // - The path name begins with a '/' => UNIX absolute path name.
    //   We insert current media name at begining
    // - For all other cases it's a relative path name
    if( m_strFile.start_with(FILE_CYGDRIVE) && m_strFile[11u] == '\\' )
    {
      // Case of paths like '/cygdrive/<letter>/'
      strDir = String::str_format("%c:\\", m_strFile[10u]);
      m_strFile = String::str_format("%c:\\%s", m_strFile[10u], m_strFile.substr(12, std::string::npos));
    }
    else if( IsSepPath(m_strFile[0u]) )
    {
      // Absolute path without drive letter:
      // - add drive letter coming from getcwd
      m_strFile = String::str_format("%c:\\%s", strDir[0u], PSZ(m_strFile));
    }
    else
    {
      // getcwd may not add the traling path separator
      if( !strDir.end_with(SEP_PATH) )
        strDir += SEP_PATH;

      if( m_strFile.is_equal(".") )
        m_strFile = strDir;
      else
        m_strFile = strDir + m_strFile;
    }
  }
}

//-------------------------------------------------------------------
// FileName::rel_name
//-------------------------------------------------------------------
String FileName::rel_name(const char* pszPath) const
{
  FileName fnRef(pszPath);

  // On windows check if both paths are on the same disk
  // Otherwise we return the full name
  if( strnicmp(PSZ(m_strFile), PSZ(fnRef.full_name()), 1) )
    return m_strFile;

  // Search for first separator. If not => return full name
  const char* p = strchr(PSZ(m_strFile), SEP_PATH);
  const char* pRef = strchr(PSZ(fnRef.full_name()), SEP_PATH);
  if (!p || !pRef)
    return m_strFile;

  String str;
  bool bClimbStarted = false;
  for(;;)
  {
    const char* p1 = strchr(p+1, SEP_PATH);
    const char* pRef1 = strchr(pRef+1, SEP_PATH);

    if( !p1 )
    {
      // No more parts in file name
      while( pRef1 )
      {
        str = String("..\\") + str;
        pRef1 = strchr(pRef1+1, SEP_PATH);
      }
      str += String(p+1);
      return str;
    }

    if( !pRef1 )
    {
      // No more reference
      str += String(p+1);
      return str;
    }

    // Compare directories
    if( (p1-p != pRef1-pRef) || bClimbStarted ||
        _strnicmp(p, pRef, p1-p) )
    {
      // Different directory
      str = String("..\\") + str;
      bClimbStarted = true;
      str.append(p+1, p1-p);
    }
    p = p1;
    pRef = pRef1;
  }
}

//----------------------------------------------------------------------------
// FileName::convert_separators
//----------------------------------------------------------------------------
void FileName::convert_separators(String *pstr)
{
  char *ptc = new char[pstr->length()+1];
  char *pStart = ptc;
  strcpy(ptc, PSZ(*pstr));

  // Convert from UNIX to DOS
  char* ptcStart = ptc;
  while (*ptc)
  {
    if (*ptc == SEP_PATHUNIX)
      *ptc = SEP_PATHDOS;

    // If there are two consecutive separators, remove the second but not if it's the begining of the path => UNC
    if (*ptc == SEP_PATHDOS && (ptc - 1) > ptcStart && *(ptc - 1) == SEP_PATHDOS)
    {
      // Removing second sep
      strcpy(ptc - 1, ptc);
    }
    ptc++;
  }

  *pstr = pStart;
  delete [] pStart;
}

//----------------------------------------------------------------------------
// FileName::mkdir
//----------------------------------------------------------------------------
void FileName::mkdir(mode_t mode, uid_t uid, gid_t gid) const throw( Exception )
{
  String str = path();
  if( str.empty() )
    return;

  char   *p;
  // Skeep UNC starting if it exists
  if( str[0u] == SEP_PATH && str[1u] == SEP_PATH )
  {
    // Saute le nom de la machine (qui suit les '\\' du debut)
    char *p1 = const_cast<char*>(strchr( PSZ(str) + 2, SEP_PATH ));
    if( p1 == NULL || p1 == PSZ(str) + 2 )
    {
      String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
      throw BadPathException(PSZ(strErr), "FileName::mkdir");
    }
    // Saute le nom du share
    p = strchr( p1 + 1, SEP_PATH );
    if( p == NULL || p == p1 + 1 )
    {
      String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
      throw BadPathException(PSZ(strErr), "FileName::mkdir");
    }
  }
  else
  {
    p = const_cast<char*>(strchr(PSZ(str), SEP_PATH));
  }

  if( !p )
  {
    String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
    throw BadPathException(PSZ(strErr), "FileName::mkdir");
  }
  p = strchr(p+1, SEP_PATH);
  if( !p )
  {
    // Check drive
    if( str[1u] != ':' )
    {
      String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
      throw BadDriveException(PSZ(strErr), "FileName::mkdir");
    }

    if( ::GetDriveType(PSZ(str)) <= 1 )
    {
      String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
      throw BadDriveException(PSZ(strErr), "FileName::mkdir");
    }
    // path = racine ; exist
    return;
  }

  do
  {
    *p = 0;
    struct stat st;
    if( ::stat(PSZ(str), &st) )
    {
      if( ::mkdir(PSZ(str)) )
      {
        // creation error
        String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
        ThrowExceptionFromErrno(PSZ(String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str))), "FileName::mkdir");
      }
    }
    else
    {
      if( !(st.st_mode & S_IFDIR) )
      {
        // c'est un fichier : erreur
        String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
        throw BadPathException(PSZ(strErr), "FileName::mkdir");
      }
      // Directory : ok
    }
    // Next path component
    *p = SEP_PATH;
    p = strchr(p+1, SEP_PATH);
  } while( p );
}

//----------------------------------------------------------------------------
// FileName::link_exist
//----------------------------------------------------------------------------
bool FileName::link_exist() const throw( Exception )
{
  //## Not implemented
  return false;
}

//----------------------------------------------------------------------------
// FileName::make_sym_link
//----------------------------------------------------------------------------
void FileName::make_sym_link(const String &strTarget, uid_t uid, gid_t gid) const throw( Exception )
{
}

//----------------------------------------------------------------------------
// FileName::size
//----------------------------------------------------------------------------
uint32 FileName::size() const throw( Exception )
{
  WIN32_FIND_DATA find;
  HANDLE h = FindFirstFile(PSZ(m_strFile), &find);
  if( h == INVALID_HANDLE_VALUE )
  {
    String strErr = String::str_format(ERR_CANNOT_FETCH_INFO, PSZ(m_strFile));
    throw Exception("FILE_ERROR", PSZ(strErr), "FileName::size");
  }
  FindClose(h);
  return (uint32)find.nFileSizeLow;
}

//----------------------------------------------------------------------------
// FileName::size64
//----------------------------------------------------------------------------
uint64 FileName::size64() const throw( Exception )
{
  WIN32_FIND_DATA find;
  HANDLE h = FindFirstFile(PSZ(m_strFile), &find);
  if( h == INVALID_HANDLE_VALUE )
  {
    String strErr = String::str_format(ERR_CANNOT_FETCH_INFO, PSZ(m_strFile));
    throw Exception("FILE_ERROR", PSZ(strErr), "FileName::size");
  }
  FindClose(h);
  return (uint64)find.nFileSizeLow;
}

//----------------------------------------------------------------------------
// FileName::mod_time
//----------------------------------------------------------------------------
void FileName::mod_time(Time *pTm, bool bLocalTime) const throw( Exception )
{
  HANDLE hFile = CreateFile(PSZ(full_name()), GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if( hFile == INVALID_HANDLE_VALUE )
  {
    String strErr = String::str_format(ERR_CANNOT_CREATE_WIN32, PSZ(m_strFile));
    throw Exception("FILE_ERROR", PSZ(strErr), "FileName::mod_time");
  }
  FILETIME fileTime;
  GetFileTime(hFile, NULL, NULL, &fileTime);
  FileTimeToLocalFileTime(&fileTime, &fileTime);
  CloseHandle(hFile);
  SYSTEMTIME sysTime;
  FileTimeToSystemTime(&fileTime, &sysTime);
  pTm->set(sysTime.wYear, (uint8)sysTime.wMonth, (uint8)sysTime.wDay,
           (uint8)sysTime.wHour, (uint8)sysTime.wMinute, 
           (double)sysTime.wSecond + sysTime.wMilliseconds/1000.);
}

//----------------------------------------------------------------------------
// FileName::set_mod_time
//----------------------------------------------------------------------------
void FileName::set_mod_time(const Time& tm) const throw( Exception )
{
  DateFields sTm;
  tm.get(&sTm);
  SYSTEMTIME sysTime;
  sysTime.wDay    = (uint16)sTm.day;
  sysTime.wYear   = (uint16)sTm.year;
  sysTime.wMonth  = (uint16)sTm.month;
  sysTime.wHour   = (uint16)sTm.hour;
  sysTime.wMinute = (uint16)sTm.min;
  sysTime.wSecond = (uint16)sTm.sec;
  sysTime.wMilliseconds = (uint16)((uint32)(sTm.sec* 1000) % 1000);
  FILETIME fileTime;
  SystemTimeToFileTime(&sysTime, &fileTime);
  LocalFileTimeToFileTime(&fileTime, &fileTime);

  int iAttribute;
  // In order to modify a directory date under Win32, we have to position the correct flag
  // otherwise windows returns a invalid handle.
  // Then we can call SetFileTime even if the handle is not a file handle 
  if( is_path_name() )
    iAttribute = FILE_FLAG_BACKUP_SEMANTICS;
  else 
    iAttribute = FILE_ATTRIBUTE_NORMAL;
  
  HANDLE hFile = CreateFile(PSZ(full_name()), GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, iAttribute, NULL);
    
  if( hFile == INVALID_HANDLE_VALUE )
  {
      String strErr = String::str_format(ERR_CANNOT_CREATE_WIN32, PSZ(m_strFile));
      throw Exception("FILE_ERROR", PSZ(strErr), "FileName::set_mod_time");
  }
  if( !SetFileTime(hFile, NULL, NULL, &fileTime) )
  {
      CloseHandle(hFile);
      String strErr = String::str_format(ERR_CANNOT_CHANGE_FILE_TIME, PSZ(m_strFile));
      throw Exception("FILE_ERROR", PSZ(strErr), "FileName::set_mod_time");
  }
  CloseHandle(hFile);
}

//-------------------------------------------------------------------
// FileName::copy
//-------------------------------------------------------------------
void FileName::copy(const String &strDst, bool bKeepMetaData) throw( Exception )
{
  if( !file_exist() )
  { // File doesn't exists
    String strErr = String::str_format(ERR_FILE_NOT_FOUND, PSZ(m_strFile));
    throw FileNotFoundException(PSZ(strErr), "FileName::copy");
  }

  FileName fDst(strDst);
  if( fDst.is_path_name() )
    // Take source name
    fDst.set(fDst.path(), name_ext());

  if (!CopyFile(PSZ(full_name()), PSZ(fDst.full_name()), FALSE))
  {
    String strErr = String::str_format(ERR_COPY_FAILED, PSZ(full_name()), PSZ(fDst.full_name()), (long)GetLastError());
    throw Exception("FILE_ERROR", PSZ(strErr), "FileName::copy");
  }
}

//-------------------------------------------------------------------
// FileName::move
//-------------------------------------------------------------------
void FileName::move(const String &strDest) throw( Exception )
{
  if( !file_exist() )
  { // File doesn't exists
    String strErr = String::str_format(ERR_FILE_NOT_FOUND, PSZ(m_strFile));
    throw FileNotFoundException(PSZ(strErr), "FileName::move");
  }

  FileName fDst(strDest);
  if( fDst.is_path_name() )
    // Take source name
    fDst.set(fDst.path(), name_ext());

  // Remove destination
  if( fDst.file_exist() )
    fDst.remove();

  copy(fDst.full_name(), true);

  // Deletes source file and changes name
  remove();
  set(fDst.full_name());
}

//-------------------------------------------------------------------
// FileName::file_system_type
//-------------------------------------------------------------------
FileName::FSType FileName::file_system_type() const throw( Exception )
{
  // Not implemented yet on windows
  return FSType(MS);
}

//-------------------------------------------------------------------
// FileName::file_system_id
//-------------------------------------------------------------------
fsid_t FileName::file_system_id() const throw( Exception )
{ 
  // Not implemented yet on windows
  return 0;
}

//-------------------------------------------------------------------
// FileName::chmod
//-------------------------------------------------------------------
void FileName::chmod(mode_t mode) throw( Exception )
{
  // Not implemented yet on windows
}

//-------------------------------------------------------------------
// FileName::chown
//-------------------------------------------------------------------
void FileName::chown(uid_t uid, gid_t gid) throw( Exception )
{
  // Not implemented yet on windows
}

//-------------------------------------------------------------------
// FileName::ThrowExceptionFromErrno
//-------------------------------------------------------------------
void FileName::ThrowExceptionFromErrno(const char *pszDesc, const char *pszOrigin) const
{
  throw Exception("FILE_ERROR", pszDesc, pszOrigin);     
}

//-------------------------------------------------------------------
// FileName::set_copy_bloc_size
//-------------------------------------------------------------------
void FileName::set_copy_bloc_size(size_t)
{
  // Nothing to do on Windows
}

//===========================================================================
// Class FileEnum
//===========================================================================

//-------------------------------------------------------------------
// FileEnum::FileEnum WIN32
//-------------------------------------------------------------------
FileEnum::FileEnum(const String &strPath, EEnumMode eMode) throw(BadPathException)
{
  // Dynamic allocation of WIN32_FIND_DATA => no need to include winbase.h
  m_pfindData = new WIN32_FIND_DATA;

  m_bFirst = true;
  m_hFind = INVALID_HANDLE_VALUE;
  init(strPath, eMode);
}

//-------------------------------------------------------------------
// FileEnum::~FileEnum WIN32
//-------------------------------------------------------------------
FileEnum::~FileEnum()
{
  close();
  delete m_pfindData;
}

//-------------------------------------------------------------------
// FileEnum::init WIN32
//-------------------------------------------------------------------
void FileEnum::init(const String &strPath, EEnumMode eMode) throw(BadPathException)
{
  close();
  m_eMode = eMode;
  m_strPath = strPath;
  m_strFile = strPath;

  // translate unix superator to win separator
  m_strPath.replace(SEP_PATHUNIX, SEP_PATHDOS);
  m_strFile.replace(SEP_PATHUNIX, SEP_PATHDOS);
}

//-------------------------------------------------------------------
// FileEnum::find() WIN32
//-------------------------------------------------------------------
bool FileEnum::find() throw(BadPathException, FileNotFoundException, Exception)
{
  String str;

  while(true)
  {
    if( m_bFirst )
    {
      str = path();
      str += '*' ;
      m_hFind = (HANDLE)FindFirstFile(PSZ(str), (WIN32_FIND_DATA *)m_pfindData);
      if( m_hFind == INVALID_HANDLE_VALUE )
        // not fount
        return false;
      m_bFirst = false;
    }
    else if( !FindNextFile((HANDLE)m_hFind, (WIN32_FIND_DATA *)m_pfindData) )
      // not found
      return false;

    str = ((WIN32_FIND_DATA *)m_pfindData)->cFileName;
    if( str.is_equal(".") == false && str.is_equal("..") == false )
    {
      // Set new file name
      set(m_strPath, str);

      // Check file type
      if( (m_eMode & ENUM_FILE) &&
          !(((WIN32_FIND_DATA *)m_pfindData)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        return true;

      // Add directory separator if needed
      if( (m_eMode & ENUM_DIR) &&
          (((WIN32_FIND_DATA *)m_pfindData)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
      {
        if( !IsSepPath(m_strFile[m_strFile.size()-1]) )
          m_strFile += (char)SEP_PATH;
        return true;
      }
    }
  }
}

//-------------------------------------------------------------------
// FileEnum::close()  WIN32
//-------------------------------------------------------------------
void FileEnum::close()
{
   if( m_hFind != INVALID_HANDLE_VALUE )
   {
     FindClose((HANDLE)m_hFind);
     m_hFind = INVALID_HANDLE_VALUE;
   }
   m_bFirst = true;
}

//-------------------------------------------------------------------
// TempFileName::TempFileName
//-------------------------------------------------------------------
TempFileName::TempFileName()
{
  set("C:\\Temp", GenerateRandomName());
}

} // namespace