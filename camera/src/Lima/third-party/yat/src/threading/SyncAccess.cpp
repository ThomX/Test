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
#include <iostream>
#include <yat/threading/SyncAccess.h>

namespace yat
{

#ifdef YAT_DEBUG
  int max_lock = 0;
  int max_obj_count = 0;
  int max_thr_count = 0;
  int thr_count = 0;
  #define DEBUG_LOG(t) std::cout << t;
  #define LOCK_STAT_MAX_COUNT(n) { if( n > max_lock ) max_lock = n; }
  #define LOCK_STAT_OBJ_COUNT(map) { if( map.size() > max_obj_count ) max_obj_count = map.size(); }
  #define THREAD_COUNT_STAT(map) {thr_count++; if( map.size() > max_thr_count ) max_thr_count = map.size(); }
  #define SHOW_STATS { std::cout << "thr count: " << thr_count << "\n" << "max simultaneous thr: " << \
                       max_thr_count << "\n" << "max locks: " << max_lock << "\n" << \
             "max obj locked: " << max_obj_count << "\n"; }
#else
  #define DEBUG_LOG(x)
  #define LOCK_STAT_MAX_COUNT(x)
  #define LOCK_STAT_OBJ_COUNT(x)
  #define THREAD_COUNT_STAT(x)
  #define SHOW_STATS
#endif

//=============================================================================
// class SyncAccess
//=============================================================================
std::map<SyncAccess::obj_ptr, SyncAccess::SyncObject *> SyncAccess::m_sSynObjMap;
yat::Mutex SyncAccess::m_sLock;
//----------------------------------------------------------------------------
// SyncAccess::SyncAccess
//----------------------------------------------------------------------------
SyncAccess::SyncAccess(void *ptr)
{
  m_pSyncObj = NULL;
  lock_obj(ptr);
}

//----------------------------------------------------------------------------
// SyncAccess::~SyncAccess
//----------------------------------------------------------------------------
SyncAccess::~SyncAccess()
{
  if( m_pSyncObj )
    unlock_obj(m_pSyncObj->m_pObj);
}

//----------------------------------------------------------------------------
// SyncAccess::lock_obj
//----------------------------------------------------------------------------
void SyncAccess::lock_obj(void *ptr)
{
  // Lock access to map
  m_sLock.lock();

  DEBUG_LOG("lock_obj: lock addr: " << ptr << " in thread: " << yat::ThreadingUtilities::self() << "\n")
  SyncObject *pSyncObj = NULL;
  // Look in map for pointer
  std::map<obj_ptr, SyncObject *>::iterator it = m_sSynObjMap.find(ptr);
  if( it == m_sSynObjMap.end() )
  {
    // Object is not referenced => create SyncObj
    pSyncObj = new SyncObject(ptr);
    m_sSynObjMap[ptr] = pSyncObj;
    m_pSyncObj = pSyncObj;
    pSyncObj->m_uiLockCount++;

    LOCK_STAT_OBJ_COUNT(m_sSynObjMap)
    DEBUG_LOG("lock_obj: new sync object. lock count: " << pSyncObj->m_uiLockCount-1 << " -> " << pSyncObj->m_uiLockCount << "\n")
  }
  else
  {
    pSyncObj = it->second;
    m_pSyncObj = pSyncObj;
    if( pSyncObj->m_CurrentThreadId == yat::ThreadingUtilities::self() )
    {
      // Current thread already lock the object
      DEBUG_LOG("lock_obj: Current thread already lock the object\n")
      
      pSyncObj->m_uiThreadCount++;
      pSyncObj->m_uiLockCount++;
      
      LOCK_STAT_MAX_COUNT(pSyncObj->m_uiLockCount)
      DEBUG_LOG("lock_obj: lock count: " << pSyncObj->m_uiLockCount-1 << " -> " << pSyncObj->m_uiLockCount << "\n")
      DEBUG_LOG("lock_obj: thr count: " << pSyncObj->m_uiThreadCount-1 << " -> " << pSyncObj->m_uiThreadCount << "\n")

      // Release access to map
      DEBUG_LOG("lock_obj: unlock sync access mutex in thread " <<  yat::ThreadingUtilities::self() << "\n")
      m_sLock.unlock();
      return;
    }
    pSyncObj->m_uiLockCount++;

    LOCK_STAT_MAX_COUNT(pSyncObj->m_uiLockCount)
    DEBUG_LOG("lock_obj: lock count = " << pSyncObj->m_uiLockCount-1 << " -> " << pSyncObj->m_uiLockCount << "\n")
  }
  
  DEBUG_LOG("lock_obj: unlock sync access mutex in thread " <<  yat::ThreadingUtilities::self() << "\n")
  
  // Release access to map to allow a unlock from another thread
  m_sLock.unlock();
  
  DEBUG_LOG("lock_obj: wait for object access\n")
  // finally lock access to object
  pSyncObj->m_Mutex.lock();
  
  DEBUG_LOG("lock_obj: Object locked in thread: " << yat::ThreadingUtilities::self() << "\n")

  // Setting thread id of the access owner
  pSyncObj->m_CurrentThreadId = yat::ThreadingUtilities::self();
  pSyncObj->m_uiThreadCount++;
}

//----------------------------------------------------------------------------
// SyncAccess::unlock_obj
//----------------------------------------------------------------------------
void SyncAccess::unlock_obj(void *ptr)
{
  // Lock access to map
  m_sLock.lock();

  DEBUG_LOG("unlock_obj: unlock addr: " << ptr << " in thread: " << yat::ThreadingUtilities::self() << "\n")

  // Look in map for pointer
  std::map<obj_ptr, SyncObject *>::iterator it = m_sSynObjMap.find(ptr);
  if( it == m_sSynObjMap.end() )
  {
    //## Object never locked => exception
    m_sLock.unlock();
    DEBUG_LOG("unlock_obj: !!!!Object never locked!!!!\n")
    return;
  }
  SyncObject *pSyncObj = it->second;
  
  pSyncObj->m_uiThreadCount--;
  pSyncObj->m_uiLockCount--;
  
  DEBUG_LOG("unlock_obj: lock count: " << pSyncObj->m_uiLockCount+1 << " -> " << pSyncObj->m_uiLockCount<< "\n")
  DEBUG_LOG("unlock_obj: thr count: " << pSyncObj->m_uiThreadCount+1 << " -> " << pSyncObj->m_uiThreadCount<< "\n")
  
  if( 0 == pSyncObj->m_uiThreadCount )
  {
    DEBUG_LOG("unlock_obj: Objet no longer in use in the current thread\n")
    // Objet no longer in use in the current thread
    pSyncObj->m_CurrentThreadId = YAT_INVALID_THREAD_UID;
    pSyncObj->m_Mutex.unlock();
  }
  
  if( 0 == pSyncObj->m_uiLockCount )
  {
    DEBUG_LOG("unlock_obj: No more LOCK() on this object => delete entry\n")
    // No more LOCK() on this object =>  delete the entry in map
    delete pSyncObj;
    m_sSynObjMap.erase(it);
  }  
  DEBUG_LOG("unlock_obj: unlock sync access mutex in thread " <<  yat::ThreadingUtilities::self() << "\n")
  m_sLock.unlock();
}


} // namespace
