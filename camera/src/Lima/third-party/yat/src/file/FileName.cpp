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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace yat
{

//===========================================================================
// FileName
//
// File name manipulations
//===========================================================================

//----------------------------------------------------------------------------
// FileName::set
//----------------------------------------------------------------------------
void FileName::set(const String &strFullName)
{
  set_full_name(PSZ(strFullName));

  if( path_exist() && !is_path_name() )
    // Add missing path separator
    m_strFile += SEP_PATH;
}

void FileName::set(const String &_strPath, const String &_strName)
{
  String strPath = _strPath;
  String strName = _strName;

  convert_separators(&strPath);
  convert_separators(&strName);

  String strFullName;

  if( strPath.end_with(SEP_PATH) )
    strFullName = strPath + strName;
  else
    strFullName = strPath + SEP_PATH + strName;
  set(strFullName);
}

void FileName::set(const String &strPath, const String &strName, const  String &strExt)
{
  set(strPath, strName + "." + strExt);
}

//----------------------------------------------------------------------------
// FileName::is_path_name
//----------------------------------------------------------------------------
bool FileName::is_path_name() const
{
  if( m_strFile.end_with(SEP_PATH) )
    return true;
  return false;
}

//----------------------------------------------------------------------------
// FileName::path
//----------------------------------------------------------------------------
String FileName::path() const
{
  String strPath = String::nil;

  // Backward search for first separator
  String::size_type iLastSepPos = m_strFile.find_last_of(SEP_PATH);
  if( String::npos != iLastSepPos )
   // Found
   strPath = m_strFile.substr(0, iLastSepPos + 1);

  return strPath;
}

//----------------------------------------------------------------------------
// FileName::name
//----------------------------------------------------------------------------
String FileName::name() const
{
  String strName = String::nil;

  // Backward search for first separator
  String::size_type iLastSepPos = m_strFile.find_last_of(SEP_PATH);
  if( String::npos == iLastSepPos )
    // If not found  : no path
    iLastSepPos = 0;

  String::size_type iExtPos = m_strFile.find_last_of(SEP_EXT);
  if( String::npos == iExtPos )
    iExtPos = m_strFile.length();

  strName = m_strFile.substr(iLastSepPos + 1, iExtPos - iLastSepPos - 1);

  return strName;
}

//----------------------------------------------------------------------------
// FileName::dir_name
//----------------------------------------------------------------------------
String FileName::dir_name() const
{
  String strName = String::nil;

  // Backward search for last separator
  String::size_type iLastSepPos = m_strFile.find_last_of(SEP_PATH);
  if ( String::npos == iLastSepPos ) {
    return String::nil;
  }
  String::size_type iPreviousSepPos = m_strFile.rfind(SEP_PATH, iLastSepPos - 1);
  if ( String::npos == iPreviousSepPos ) {
    return m_strFile.substr(0, iLastSepPos);
  }
  return m_strFile.substr(iPreviousSepPos + 1, iLastSepPos - iPreviousSepPos - 1);
}

//----------------------------------------------------------------------------
// FileName::name_ext
//----------------------------------------------------------------------------
String FileName::name_ext() const
{
  String strFileName = name();
  if( ext().size() > 0 )
    strFileName += '.' + ext();
  return strFileName;
}

//----------------------------------------------------------------------------
// FileName::ext
//----------------------------------------------------------------------------
String FileName::ext() const
{
  String strExt = String::nil;

  // Search backward for extension separator
  String::size_type iExtPos = m_strFile.find_last_of(SEP_EXT);
  if( String::npos != iExtPos )
    // Separator found
    strExt = m_strFile.substr(iExtPos + 1);

  return strExt;
}

//----------------------------------------------------------------------------
// FileName::remove
//----------------------------------------------------------------------------
void FileName::remove() throw( Exception )
{
  if( !m_strFile.empty() )
  {
    if( unlink(PSZ(full_name())) )
    {
      String strErr = String::str_format(ERR_CANNOT_REMOVE_FILE, PSZ(full_name()));
      ThrowExceptionFromErrno(PSZ(strErr), "FileName::remove");
    }
  }
}

//----------------------------------------------------------------------------
// FileName::rmdir
//----------------------------------------------------------------------------
void FileName::rmdir(bool bRecursive, bool bContentOnly) throw( Exception )
{
  if( !is_path_name() )
    throw BadPathException(PSZ(String::str_format(ERR_DELETE_DIRECTORY, PSZ(full_name()))), "FileName::rmdir");

  if( !m_strFile.empty() )
  {
    if( !bRecursive )
    {
      // Delete files & symbolic links
      FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
      while( fileEnum.find() )
        fileEnum.remove();
      if( !bContentOnly )
      {
        if( ::rmdir(PSZ(full_name())) )
        {
          String strErr = String::str_format(ERR_CANNOT_REMOVE_FILE, PSZ(full_name()));
          ThrowExceptionFromErrno(PSZ(strErr), "FileName::rmdir");
        }
      }
    }
    else
    { // Recursively delete directory content
      FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR);
      while( dirEnum.find() )
        dirEnum.rmdir(true);
      // Delete files & symbolic links
      FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
      while( fileEnum.find() )
        fileEnum.remove();

      if( !bContentOnly )
      {
        // And finally the direcory himself
        rmdir();
      }
    }
  }
}

//----------------------------------------------------------------------------
// FileName::rename
//----------------------------------------------------------------------------
void FileName::rename(const String &strNewName) throw( Exception )
{
  if( !m_strFile.empty() )
    if( ::rename(PSZ(m_strFile), PSZ(strNewName)) )
    {
      String strErr = String::str_format(ERR_CANNOT_RENAME_FILE, PSZ(m_strFile));
      ThrowExceptionFromErrno(PSZ(strErr), "FileName::rename");
    }
 
  // Change internal name
  set(strNewName);
}

//-------------------------------------------------------------------
// FileName::dir_copy
//-------------------------------------------------------------------
void FileName::dir_copy(const String &strDest, bool bCreateDir, mode_t modeDir, uid_t uid, gid_t gid) throw( Exception )
{
  FileName fnDst;

  // Create destination path
  fnDst.set(strDest);
  fnDst.mkdir(modeDir, uid, gid);

  if( bCreateDir )
  {
    // Create source directory name inside destination path
    fnDst.set(strDest + dir_name() + SEP_PATH);
    fnDst.mkdir(modeDir, uid, gid);
  }

  if( !fnDst.is_path_name() )
    throw BadPathException(PSZ(String::str_format(ERR_BAD_DEST_PATH, PSZ(fnDst.full_name()))),
                           "FileName::dir_copy");

  // Recursively copying sub-directories
  FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR);
  while( dirEnum.find() )
    dirEnum.dir_copy(fnDst.path(), true, modeDir, uid, gid);

  // Copying directory files
  FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
  while( fileEnum.find() )
    // Copy with metadata
    fileEnum.copy(fnDst.path(), true);
}

//-------------------------------------------------------------------
// FileName::recursive_chmod
//-------------------------------------------------------------------
void FileName::recursive_chmod(mode_t modeFile, mode_t modeDir, bool bCurrentLevel) throw( Exception )
{
  if( !path_exist() )
  { // File doesn't exists
    String strErr = String::str_format(ERR_DIR_NOT_FOUND, PSZ(full_name()));
    throw FileNotFoundException(PSZ(strErr), "FileName::recursive_chmod");
  }

  // Recursively change rights on sub-directories
  FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR);
  while( dirEnum.find() )
    dirEnum.recursive_chmod(modeFile, modeDir, true);

  // Change mode on files
  FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
  while( fileEnum.find() )
    // Copy with metadata
    fileEnum.chmod(modeFile);

  if( bCurrentLevel )
    // Change mode to directory itself
    chmod(modeDir);
}

//-------------------------------------------------------------------
// FileName::recursive_chmod_file
//-------------------------------------------------------------------
void FileName::recursive_chmod_file(mode_t mode) throw( Exception )
{
  if( !path_exist() )
  { // File doesn't exists
    String strErr = String::str_format(ERR_DIR_NOT_FOUND, PSZ(full_name()));
    throw FileNotFoundException(PSZ(strErr), "FileName::recursive_chmod_file");
  }

  // Recursively change rights on sub-directories
  FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR);
  while( dirEnum.find() )
    dirEnum.recursive_chmod_file(mode);

  // Change mode on files
  FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
  while( fileEnum.find() )
    // Copy with metadata
    fileEnum.chmod(mode);
}

//-------------------------------------------------------------------
// FileName::recursive_chmod_dir
//-------------------------------------------------------------------
void FileName::recursive_chmod_dir(mode_t mode) throw( Exception )
{
  if( !path_exist() )
  { // File doesn't exists
    String strErr = String::str_format(ERR_DIR_NOT_FOUND, PSZ(full_name()));
    throw FileNotFoundException(PSZ(strErr), "FileName::recursive_chmod_dir");
  }

  // Recursively change rights on sub-directories
  FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR);
  while( dirEnum.find() )
    dirEnum.recursive_chmod_dir(mode);

  // Change mode to directory itself
  chmod(mode);
}

//-------------------------------------------------------------------
// FileName::recursive_chown
//-------------------------------------------------------------------
void FileName::recursive_chown(uid_t uid, gid_t gid) throw( Exception )
{
  if( !path_exist() )
  { // File doesn't exists
    String strErr = String::str_format(ERR_DIR_NOT_FOUND, PSZ(full_name()));
    throw FileNotFoundException(PSZ(strErr), "FileName::recursive_chmod");
  }

  // Recursively change rights on sub-directories
  FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR);
  while( dirEnum.find() )
    dirEnum.recursive_chmod(uid, gid);

  // Change mode on files
  FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
  while( fileEnum.find() )
    // Copy with metadata
    fileEnum.chown(uid, gid);

  // Change mode to directory itself
  chown(uid, gid);
}

//-------------------------------------------------------------------
// FileName::is_empty_dir
//-------------------------------------------------------------------
bool FileName::is_empty_dir() const
{
  FileEnum dirEnum(full_name(), FileEnum::ENUM_DIR );
  FileEnum fileEnum(full_name(), FileEnum::ENUM_FILE);
  
  if( !dirEnum.find() && !fileEnum.find() )
  return true;
  return false;
}

//===========================================================================
// Class TempFileName
//===========================================================================
long TempFileName::s_lLastNumber = 0;

//-------------------------------------------------------------------
// TempFileName::GenerateRandomName
//-------------------------------------------------------------------
String TempFileName::GenerateRandomName()
{
  if( !s_lLastNumber )
    // Initialize random sequence
    s_lLastNumber = CurrentTime().long_unix();

  return String::str_format("temp%lx", s_lLastNumber++);
}

//-------------------------------------------------------------------
// TempFileName::TempFileName(path)
//-------------------------------------------------------------------
TempFileName::TempFileName(const String &strPath)
{
  set(strPath, GenerateRandomName());
}

//===========================================================================
// Class File
//===========================================================================

//-------------------------------------------------------------------
// File::load(MemBuf)
//-------------------------------------------------------------------
void File::load(MemBuf *pBuf) throw(Exception)
{
  // Open source file
  FILE *fi = fopen(PSZ(full_name()), "r");
  if( NULL == fi )
  {
    String strErr = String::str_format(ERR_OPEN_FILE, PSZ(full_name()));
    throw Exception("FILE_ERROR", PSZ(strErr), "File::Load");
  }

  // Buffer
  pBuf->set_len(size()+1);

  // Read
  long lSize = size();
  long lReaded = fread(pBuf->buf(), 1, lSize, fi);
  if( ferror(fi) || lSize != lReaded )
  {
    String strErr = String::str_format(ERR_READING_FILE, PSZ(full_name()));
    throw Exception("FILE_ERROR", PSZ(strErr), "File::Load");
  }
  memset(pBuf->buf() + lSize, 0, 1);
  fclose(fi);
}

//-------------------------------------------------------------------
// File::load(String*)
//-------------------------------------------------------------------
void File::load(String *pString) throw(Exception)
{
  MemBuf buf;
  load(&buf);
  buf >> (*pString);
}

//-------------------------------------------------------------------
// File::save
//-------------------------------------------------------------------
void File::save(const String &strContent) throw(Exception)
{
  // Open destination file
  FILE *fi = fopen(PSZ(full_name()), "wb");
  if( NULL == fi )
  {
    String strErr = String::str_format(ERR_OPEN_FILE, PSZ(full_name()));
    throw Exception("FILE_ERROR", PSZ(strErr), "File::Load");
  }

  // Write text content
  int iRc = fputs(PSZ(strContent), fi);
  if( EOF == iRc )
  {
    String strErr;
    strErr.printf("Cannot write in file '%s'", PSZ(full_name()));
    throw Exception("FILE_ERROR", PSZ(strErr), "File::Save");
  }
  fclose(fi);
}

//===========================================================================
// class Cfgfile
//===========================================================================

//-----------------------------------------------------------------------------
// CfgFile::CfgFile
//-----------------------------------------------------------------------------
CfgFile::CfgFile(const String &strFile) : File(strFile) { }

//-----------------------------------------------------------------------------
// CfgFile::load
//-----------------------------------------------------------------------------
void CfgFile::load() throw(Exception)
{
  try
  {
    String strContent;
    File::load(&strContent);
    String strLine;

    // Create a 'default' section
    Section aSection;
    m_strSection = "default";
    m_dictSection[m_strSection] = aSection;

    while( !strContent.empty() )
    {
      // Extract next line
      strContent.extract_token('\n', &strLine);
      // Supress blank characters at the begining and the end of the string
      strLine.trim();
      // Pass blank line and comments line
      if( strLine.match("#*") || strLine.empty() )
        continue;

      if( strLine.match("[*]") )
      {  // Section declaration
        Section aSection;
        strLine.extract_token('[', ']', &m_strSection);
        m_strSection.trim();
        m_dictSection[m_strSection] = aSection;
        continue;
      }

      if( strLine.match("*=*") )
      {
        String strParamName;
        strLine.extract_token('=', &strParamName);
        strLine.trim();
        strParamName.trim();
        m_dictSection[m_strSection].m_dictParameters[strParamName] = strLine;

      }
      else
        m_dictSection[m_strSection].m_vecSingleValues.push_back(strLine);
    }
  }
  catch (Exception &)
  {
    throw;
  }
  // Set cursor on default section
  m_strSection = "default";
}

//-----------------------------------------------------------------------------
// CfgFile::get_values
//-----------------------------------------------------------------------------
const CfgFile::Values &CfgFile::get_values(const String &strSection)
{
  if( strSection.empty() )
    return m_dictSection[m_strSection].m_vecSingleValues;
  else
    return m_dictSection[strSection].m_vecSingleValues;
}

//-----------------------------------------------------------------------------
// CfgFile::get_parameters
//-----------------------------------------------------------------------------
const CfgFile::Parameters &CfgFile::get_parameters()
{
  return m_dictSection[m_strSection].m_dictParameters;
}

//-----------------------------------------------------------------------------
// CfgFile::GetParamValue
//-----------------------------------------------------------------------------
String CfgFile::get_param_value(const String &strParamName)
{
  CfgFile::Parameters::const_iterator cit =
          m_dictSection[m_strSection].m_dictParameters.find(strParamName);

  if( cit == m_dictSection[m_strSection].m_dictParameters.end() )
    return String::nil;

  return cit->second;
}

//-----------------------------------------------------------------------------
// CfgFile::has_parameter
//-----------------------------------------------------------------------------
bool CfgFile::has_parameter(const String &strParamName)
{
  CfgFile::Parameters::const_iterator cit =
          m_dictSection[m_strSection].m_dictParameters.find(strParamName);

  return cit != m_dictSection[m_strSection].m_dictParameters.end() ? true : false;
}

//-----------------------------------------------------------------------------
// CfgFile::set_section
//-----------------------------------------------------------------------------
bool CfgFile::set_section(const String &strSection, bool bThrowException) throw( Exception )
{
  std::map<String, Section>::iterator it = m_dictSection.find(strSection);
  if( it != m_dictSection.end() )
    m_strSection = strSection;
  else if( bThrowException )
      throw Exception("NO_DATA", PSZ(String::str_format("Section '%s' not found", PSZ(strSection))), "CfgFile::SetSection");
  else
    return false;
  return true;
}

//===========================================================================
// Class DirectoryWatcher
//===========================================================================

//-----------------------------------------------------------------------------
// DirectoryWatcher::Entry::Entry
//-----------------------------------------------------------------------------
DirectoryWatcher::Entry::Entry(const String &strFullName)
{
  ptrFile = new FileName(strFullName);
  ptrFile->mod_time(&tmLastModTime);
  bRemoved = false;
}

//-----------------------------------------------------------------------------
// DirectoryWatcher::DirectoryWatcher
//-----------------------------------------------------------------------------
DirectoryWatcher::DirectoryWatcher(const String &strDirectoryPath, WatchMode eMode) throw(Exception)
{
  set(strDirectoryPath);
  m_tmDirModTime.set_long_unix(0);
  m_bDirectoryHasChanged = false;
  m_eMode = eMode;
  
  if( ENUM_FIRST == eMode )
  {
    FileEnum fe(full_name(), FileEnum::ENUM_ALL);
    while( fe.find() )
      m_mapEntry[fe.full_name().hash64()] = new Entry(fe.full_name());
  }
}

//-----------------------------------------------------------------------------
// DirectoryWatcher::priv_has_changed
//-----------------------------------------------------------------------------
bool DirectoryWatcher::priv_has_changed(bool bReset) 
{
  CurrentTime tmCur;
  Time tmModTime;
  mod_time(&tmModTime);
  
  if( m_tmDirModTime.is_empty() && NO_FIRST_ENUM == m_eMode )
  {   // First call
    m_tmLocalModTime = tmCur;
    m_tmDirModTime = tmModTime;
    return true;
  }
  else if( tmModTime != m_tmDirModTime && !m_bDirectoryHasChanged )
  {
    m_bDirectoryHasChanged = true;
    m_tmLocalModTime = tmCur;
    m_tmDirModTime = tmModTime;
    return false;
  }
        // The change is considered at least 1 second after its first observation
  else if( m_bDirectoryHasChanged && tmCur.double_unix() - m_tmLocalModTime.double_unix() > 1.0 )
  {
    if( bReset )
    {
      if( tmModTime == m_tmDirModTime )
        m_bDirectoryHasChanged = false;
      else
      {
        m_tmLocalModTime = tmCur;
        m_tmDirModTime = tmModTime;
      }
    }
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
// DirectoryWatcher::has_changed
//-----------------------------------------------------------------------------
bool DirectoryWatcher::has_changed()
{
  return priv_has_changed(false);
}

//-----------------------------------------------------------------------------
// DirectoryWatcher::get_changes
//-----------------------------------------------------------------------------
void DirectoryWatcher::get_changes(FileNamePtrVector *pvecNewFilesPtr, 
                                  FileNamePtrVector *pvecChangedFileNamePtr,
                  FileNamePtrVector *pvecRemovedFileNamePtr) throw(Exception)
{
  Time tmModTimeDir;
  
  // File modifications does not affect the last modification time of its parent directory
  if( pvecChangedFileNamePtr || priv_has_changed(true) )
  {
    if( pvecRemovedFileNamePtr )
    {
      // Mark all entry as removed for detection of removed entries
      for( EntryMap::iterator it = m_mapEntry.begin(); it != m_mapEntry.end(); it++ )
        it->second->bRemoved = true;
    }

    FileEnum fe(full_name(), FileEnum::ENUM_ALL);
    while( fe.find() )
    {
      uint64 hashFile = fe.full_name().hash64();
      
      if( m_mapEntry.find(hashFile) != m_mapEntry.end() )
      {
        EntryPtr ptrEntry = m_mapEntry[hashFile];
        ptrEntry->bRemoved = false;
        
        // Existing file, check for change if needed
        if( pvecChangedFileNamePtr )
        {
          Time tmModTime;
          fe.mod_time(&tmModTime);

          if( ptrEntry->tmLastModTime != tmModTime )
          {
            ptrEntry->tmLastModTime = tmModTime;
            pvecChangedFileNamePtr->push_back(ptrEntry->ptrFile);
          }
        }
      }
      else if( pvecNewFilesPtr )
      {
        m_mapEntry[hashFile] = new Entry(fe.full_name());
        pvecNewFilesPtr->push_back(m_mapEntry[hashFile]->ptrFile);
      }      
    }
    
    if( pvecRemovedFileNamePtr )
    {
      // Delete all removed entry and push them in the user vector
      for( EntryMap::iterator it = m_mapEntry.begin(); it != m_mapEntry.end(); )
      {
        if( it->second->bRemoved )
        {
          EntryMap::iterator itErase = it;
          it++;
          pvecRemovedFileNamePtr->push_back(itErase->second->ptrFile);
          m_mapEntry.erase(itErase);
        }
        else
          it++;
      }
    }
  }
}

} // namespace
