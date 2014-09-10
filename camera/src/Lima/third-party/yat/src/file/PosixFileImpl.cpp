//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2011  The Tango Community
//
// Part of the code comes from the ACE Framework (i386 asm bytes swaping code)
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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace yat
{

///===========================================================================
/// FileName
///===========================================================================
size_t FileName::s_copy_bloc_size = 1048576;

//----------------------------------------------------------------------------
// FileName::access_from_string
//----------------------------------------------------------------------------
mode_t FileName::access_from_string(const String &strAccess)
{
  mode_t mode = 0;
  sscanf(PSZ(strAccess), "%o", &mode);
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

  struct stat  st ;
  int          iRc ;

  if( uiLen>=2 && IsSepPath(pszPath[uiLen-1]) && pszPath[uiLen-2] != ':' )
  {
    // Path ends with '\' => remove '\'
    String strPath = pszPath;
    strPath = strPath.substr(0, strPath.size()-1);
    iRc = stat(PSZ(strPath), &st);
    if( iRc && errno != ENOENT )
      ThrowExceptionFromErrno(PSZ(String::str_format(ERR_STAT_FAILED, PSZ(strPath))), "FileName::path_exist");
  }
  else
  {
    iRc = stat(pszPath, &st);
    if( iRc && errno != ENOENT )
      ThrowExceptionFromErrno(PSZ(String::str_format(ERR_STAT_FAILED, pszPath)), "FileName::path_exist");
  }
  return !iRc && (st.st_mode & S_IFDIR);
}

//-------------------------------------------------------------------
// FileName::file_exist
//-------------------------------------------------------------------
bool FileName::file_exist() const
{
  pcsz pcszfull_name = PSZ(full_name());

  struct stat st;
  return (!access(pcszfull_name, F_OK) &&
        !stat(pcszfull_name, &st) &&
         (st.st_mode & S_IFREG));
}

//-------------------------------------------------------------------
// FileName::file_access
//-------------------------------------------------------------------
bool FileName::file_access() const
{
  return !access(PSZ(full_name()), F_OK);
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

  if (IsSepPath(strFileName[0u]))
  {
    // Absolute name
    m_strFile = strFileName;
  }
  else
  {
    // relative name: add current working directory
    char cbuf[_MAX_PATH];
    /* char* ignored = */ getcwd(cbuf, _MAX_PATH);
    m_strFile = String::str_format("%s/%s", cbuf, PSZ(strFileName));
  }

}

//-------------------------------------------------------------------
// FileName::rel_name
//-------------------------------------------------------------------
String FileName::rel_name(const char* pszPath) const
{
  FileName fnRef(pszPath);

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
        str = String("../") + str;
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
        // Unix : le case est important
        strncmp(p, pRef, p1-p) )
    {
      // Different directory
      str = String("../") + str;
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


  // Convert from DOS to UNIX
  while (*ptc)
  {
    if (*ptc == SEP_PATHDOS)
      *ptc = SEP_PATHUNIX;
    ptc++;
  }

  *pstr = pStart;
  delete [] pStart;
}

//----------------------------------------------------------------------------
// FileName::mkdir
//----------------------------------------------------------------------------
void FileName::mkdir(mode_t mode, uid_t uid, gid_t gid) const
  throw( Exception )
{
  String str = path();
  if( str.empty() )
    return;

  char *p;
  p = ::strchr(const_cast<char*>(PSZ(str)), SEP_PATH);

  if( !p )
  {
    String strErr = String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str));
    throw BadPathException(PSZ(strErr), "FileName::mkdir");
  }
  p = strchr(p+1, SEP_PATH);
  if( !p )
  {
    // path = racine ; exist
    return;
  }

  do
  {
    *p = 0;
    struct stat st;
    if( ::stat(PSZ(str), &st) )
    {
      if( errno != ENOENT )
        // stat call report error != file not found
        ThrowExceptionFromErrno(PSZ(String::str_format(ERR_STAT_FAILED, PSZ(str))), "FileName::mkdir");

      if( ::mkdir(PSZ(str), 0000777) )
      {
        // failure
        ThrowExceptionFromErrno(PSZ(String::str_format(ERR_CANNOT_CREATE_FOLDER, PSZ(str))), "FileName::mkdir");
      }

      // Change access mode if needed
      if( mode != 0 )
      {
        if( ::chmod(PSZ(str), mode) )
        {
          // changing access mode failed
          ThrowExceptionFromErrno(PSZ(String::str_format(ERR_CHMOD_FAILED, PSZ(str), mode)), "FileName::mkdir");
        }
      }
      // Change owner if needed
      if( (int)uid != -1 || (int)gid != -1 )
      {
        if( ::chown(PSZ(str), uid, gid) )
        {
          // changing owner mode failed
          ThrowExceptionFromErrno(PSZ(String::str_format(ERR_CHOWN_FAILED, PSZ(str), uid, gid)), "FileName::mkdir");
        }
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
  struct stat st;
  String strFullName = full_name();
  if( is_path_name() )
    strFullName.erase(strFullName.size()-1, 1);
  int iRc = lstat(PSZ(strFullName), &st);
  if( !iRc && S_ISLNK(st.st_mode) )
    return true;

/* Probably stupid... no file => no link!
  else if( iRc )
  {
    String strErr = String::str_format(ERR_TEST_LINK, PSZ(full_name()));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::link_exist");
  }
*/

  return false;
}

//----------------------------------------------------------------------------
// FileName::make_sym_link
//----------------------------------------------------------------------------
void FileName::make_sym_link(const String &strTarget, uid_t uid, gid_t gid) const throw( Exception )
{
  int iRc = symlink(PSZ(strTarget), PSZ(full_name()));
  if( iRc )
  {
    String strErr = String::str_format(ERR_CANNOT_CREATE_LINK, PSZ(full_name()), PSZ(strTarget));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::make_sym_link");
  }
  // Change owner if needed
  if( (int)uid != -1 || (int)gid != -1 )
  {
    if( lchown(PSZ(full_name()), uid, gid) )
    {
      // changing owner mode failed
      String strErr = String::str_format(ERR_CHOWN_FAILED, PSZ(full_name()), uid, gid);
      ThrowExceptionFromErrno(PSZ(strErr), "FileName::make_sym_link");
    }
  }
}

//----------------------------------------------------------------------------
// FileName::size
//----------------------------------------------------------------------------
uint32 FileName::size() const throw( Exception )
{
  struct stat sStat;
  if( stat(PSZ(full_name()), &sStat) == -1 )
  {
    String strErr = String::str_format(ERR_CANNOT_FETCH_INFO, PSZ(m_strFile));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::size");
  }
  return sStat.st_size;
}

//----------------------------------------------------------------------------
// FileName::size64
//----------------------------------------------------------------------------
uint64 FileName::size64() const throw( Exception )
{
  struct stat64 sStat;
  if( stat64(PSZ(full_name()), &sStat) == -1 )
  {
    String strErr = String::str_format(ERR_CANNOT_FETCH_INFO, PSZ(m_strFile));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::size");
  }
  return sStat.st_size;
}

//----------------------------------------------------------------------------
// FileName::mod_time
//----------------------------------------------------------------------------
void FileName::mod_time(Time *pTm, bool bLocalTime) const throw( Exception )
{
  struct stat sStat;
  if( stat(PSZ(full_name()), &sStat) == -1 )
  {
    String strErr = String::str_format(ERR_CANNOT_GET_FILE_TIME, PSZ(m_strFile));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::mod_time");
  }

  if( bLocalTime )
  {
    struct tm tmLocal;
    localtime_r(&sStat.st_mtime, &tmLocal);
    pTm->set_long_unix(mktime(&tmLocal) + tmLocal.tm_gmtoff);
  }
  else
    pTm->set_long_unix(sStat.st_mtime);
}

//----------------------------------------------------------------------------
// FileName::set_mod_time
//----------------------------------------------------------------------------
void FileName::set_mod_time(const Time& tm) const throw( Exception )
{
  struct utimbuf sTm;
  struct stat sStat;

  if( stat(PSZ(full_name()), &sStat) != -1 )
    // Get access time, in order to preserve it
    sTm.actime = sStat.st_atime;
  else
    // stat function failed, use the new mod time
    sTm.actime = tm.long_unix();

  sTm.modtime = tm.long_unix();
  if( utime(PSZ(full_name()), &sTm) )
  {
    String strErr = String::str_format(ERR_CANNOT_CHANGE_FILE_TIME, PSZ(m_strFile));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::set_mod_time");
  }
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

  // Self copy ?
  if( full_name().is_equal(fDst.full_name()) )
  {
    String strErr = String::str_format(ERR_COPY_FAILED, PSZ(full_name()), PSZ(fDst.full_name()));
    throw Exception("FILE_ERROR", PSZ(strErr), "FileName::copy");
  }

  struct stat st;
  // keep metadata & Is root  ?
  if( bKeepMetaData && 0 == geteuid() )
  {
    int iRc = stat(PSZ(full_name()), &st);
    if( iRc )
    {
      String strErr = String::str_format(ERR_COPY_FAILED, PSZ(full_name()), PSZ(fDst.full_name()));
      ThrowExceptionFromErrno(PSZ(strErr), "FileName::copy");
    }
  }

  // Open source file
  int fsrc = open(PSZ(full_name()), O_RDONLY);
  if( fsrc < 0 )
  {
    String strErr = String::str_format(ERR_OPEN_FILE, PSZ(full_name()));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::copy");
  }

  // Buffer
  char aBuf[s_copy_bloc_size];

  // Get last modified time
  Time tmLastMod;
  mod_time(&tmLastMod);

  // Opens destination file
  int fdst = creat(PSZ(fDst.full_name()), st.st_mode);
  if( fdst < 0 )
  {
    String strErr = String::str_format(ERR_OPEN_FILE, PSZ(fDst.full_name()));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::copy");
  }

  try
  {
    // Copy by blocs
    int64 llSize = size64();
    ssize_t lReaded=0, lWritten=0;
    while( llSize )
    {
      size_t lToRead = 0;

      if( llSize > s_copy_bloc_size )
        lToRead = s_copy_bloc_size;
      else
        lToRead = (size_t)llSize;

      lReaded = read(fsrc, aBuf, lToRead);
      if( lReaded < 0 )
      {
        String strErr = String::str_format(ERR_READING_FILE, PSZ(full_name()));
        ThrowExceptionFromErrno(PSZ(strErr), "FileName::copy");
      }

      lWritten = write(fdst, aBuf, lToRead);
      if( lWritten < 0 )
      {
        String strErr = String::str_format(ERR_WRITING_FILE, PSZ(fDst.full_name()));
        ThrowExceptionFromErrno(PSZ(strErr), "FileName::copy");
      }

      llSize -= lWritten;
    }
  }
  catch( yat::Exception &ex )
  {
    close(fsrc);
    close(fdst);
    throw ex;
  }
  
  close(fsrc);
  close(fdst);

  // Copy last modifitation date
  fDst.set_mod_time(tmLastMod);

  // if root copy file metadata: access mode, owner & group
  if( bKeepMetaData && 0 == geteuid() )
  {
    try
    {
      fDst.chown(st.st_uid, st.st_gid);
    }

    catch( ... )
    {
      // Don't care, we did our best effort...
    }
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

  // Check filesystem id, if it's the same, we can try to rename file
  fsid_t idSrc = file_system_id();
  fsid_t idDst = fDst.file_system_id();
  if( idSrc.__val[0] == idDst.__val[0] && idSrc.__val[1] == idDst.__val[1] )
  {
    try
    {
      rename(fDst.full_name());
    }
    catch( Exception e )
    {
      // Unable to rename => make a copy
      copy(fDst.full_name(), true);

      // Deletes source file and changes name
      remove();
      set(fDst.full_name());
    }
  }
  else
  {
    copy(fDst.full_name(), true);

    // Deletes source file and changes name
    remove();
    set(fDst.full_name());
  }
}

//-------------------------------------------------------------------
// FileName::file_system_type
//-------------------------------------------------------------------
FileName::FSType FileName::file_system_type() const throw( Exception )
{
  struct statfs buf;
  int iRc = statfs(PSZ(path()), &buf);
  if( iRc )
  {
    String strErr = String::str_format(ERR_FSTYPE, PSZ(path()));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::file_system_type");
  }
  return FSType(buf.f_type);
}

//-------------------------------------------------------------------
// FileName::file_system_id
//-------------------------------------------------------------------
fsid_t FileName::file_system_id() const throw( Exception )
{
  struct statfs buf;
  int iRc = statfs(PSZ(path()), &buf);
  if( iRc )
  {
    String strErr = String::str_format(ERR_FSTYPE, PSZ(path()));
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::file_system_type");
  }
  return buf.f_fsid;
}

//-------------------------------------------------------------------
// FileName::chmod
//-------------------------------------------------------------------
void FileName::chmod(mode_t mode) throw( Exception )
{
  int iRc = ::chmod(PSZ(full_name()), mode);
  if( iRc )
  {
    String strErr = String::str_format(ERR_CHMOD_FAILED, PSZ(full_name()), mode);
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::chmod");
  }
}

//-------------------------------------------------------------------
// FileName::chown
//-------------------------------------------------------------------
void FileName::chown(uid_t uid, gid_t gid) throw( Exception )
{
  int iRc = ::chown(PSZ(full_name()), uid, gid);
  if( iRc )
  {
    String strErr = String::str_format(ERR_CHOWN_FAILED, PSZ(full_name()), uid, gid);
    ThrowExceptionFromErrno(PSZ(strErr), "FileName::chown");
  }
}

//-------------------------------------------------------------------
// FileName::ThrowExceptionFromErrno
//-------------------------------------------------------------------
void FileName::ThrowExceptionFromErrno(const char *pszDesc, const char *pszOrigin) const
{
  String strDesc = String::str_format("%s. %s", pszDesc, strerror(errno));
  switch( errno )
  {
    case EIO:
      throw IOException(PSZ(strDesc), pszOrigin);
    case EPERM:
    case EACCES:
    case EROFS:
      throw PermissionException(PSZ(strDesc), pszOrigin);
    case ENOTEMPTY:
      throw BadPathConditionException(PSZ(strDesc), pszOrigin);
    case ENAMETOOLONG:
    case ELOOP:
    case EISDIR:
    case ENOTDIR:
    case EBADF:
      throw BadPathException(PSZ(strDesc), pszOrigin);
    case ENOENT:
      throw FileNotFoundException(PSZ(strDesc), pszOrigin);
    default:
      throw Exception("FILE_ERROR", PSZ(strDesc), pszOrigin);
  }
}

//-------------------------------------------------------------------
// FileName::set_copy_bloc_size
//-------------------------------------------------------------------
void FileName::set_copy_bloc_size(size_t size)
{
  s_copy_bloc_size = size;
}


//===========================================================================
// Class FileEnum
//===========================================================================

//-------------------------------------------------------------------
// Initialisation
//-------------------------------------------------------------------
FileEnum::FileEnum(const String &strPath, EEnumMode eMode) throw(BadPathException)
{
  m_dirDir = NULL;
  init(strPath, eMode);
}

//-------------------------------------------------------------------
// Destructeur
//-------------------------------------------------------------------
FileEnum::~FileEnum()
{
  close();
}

//-------------------------------------------------------------------
// FileEnum::init
//-------------------------------------------------------------------
void FileEnum::init(const String &strPath, EEnumMode eMode) throw(BadPathException)
{
  close();
  m_eMode = eMode;
  set(PSZ(strPath));

  // Initialize enumeration
  m_dirDir = opendir(PSZ(path()));
  if( NULL == m_dirDir )
  {
    String strErr = String::str_format(ERR_CANNOT_ENUM_DIR, PSZ(path()));
    throw BadPathException(PSZ(strErr), "FileEnum::Init");
  }

  m_strPath = strPath; // Save initial path.

  // translate win separator to unix superator
  m_strPath.replace(SEP_PATHDOS, SEP_PATHUNIX);
}

//-------------------------------------------------------------------
// FileEnum::find
//-------------------------------------------------------------------
bool FileEnum::find() throw(BadPathException, FileNotFoundException, Exception)
{
  struct dirent *dirEntry;
  String str;
  while( (dirEntry = readdir(m_dirDir)) != NULL )
  {
    str = dirEntry->d_name;
    if( str.is_equal(".") == false && str.is_equal("..") == false )
    {
      // Set new file name
      set(m_strPath, dirEntry->d_name);

      // Check file
      if( (m_eMode & ENUM_FILE) && file_exist() )
        return true;
      if( (m_eMode & ENUM_DIR) && path_exist() )
        return true;
    }
  }

  // Not found
  return false;
}

//-------------------------------------------------------------------
// FileEnum::close
//-------------------------------------------------------------------
void FileEnum::close()
{
  if(m_dirDir)
    closedir(m_dirDir);
}

//-------------------------------------------------------------------
// TempFileName::TempFileName
//-------------------------------------------------------------------
TempFileName::TempFileName()
{
  set("/tmp", GenerateRandomName());
}

} // namespace
