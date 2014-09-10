/*!
 * \file     
 * \brief    An advanced yat example
 * \author   N. Leclercq - Synchrotron SOLEIL
 */

#pragma once

// ============================================================================
// Dependencies
// ============================================================================
#include <iostream>
#include <yat/memory/Allocator.h>

// ============================================================================
// class: Object
// ============================================================================
class Object
{   
  //- make the yat::NewAllocator a friend of Object
  friend class yat::NewAllocator<Object>;
  
public:
  //- init the underlying memory cache
  static void init (size_t nb = 10, size_t bs = 1000)
  {
    cache = new yat::CachedAllocator<Object>(nb, bs);
    std::cout << "Object::init::cache contains " << cache->length() << " objects" << std::endl;
    std::cout << "Object::init::cache contains " << cache->size() / (1024. * 1024.) << " Mb" << std::endl;
  }
  
  //- overwrites the new operator
  inline static Object * instanciate ()
  {
    //- std::cout << "Object::instanciate" << std::endl; 
    return cache->malloc();
  }
  
  //- overwrites the new operator
  inline static void release (Object * p)
  {
    //- std::cout << "Object::release" << std::endl; 
    cache->free(p);
  }
  
  //- release the underlying memory cache
  static void fini ()
  {
    std::cout << "Object::fini::cache contains " << cache->length() << " objects" << std::endl;
    std::cout << "Object::init::cache contains " << cache->size() / (1024. * 1024.) << " Mb" << std::endl;
    delete cache;
    cache = 0;
  }
 
  //-------------------------------------------------
  // Instance Id
  //-------------------------------------------------
  const size_t inst_id;
  
private:
  Object ()
    : inst_id (++Object::inst_cnt)
  {
    //- std::cout << "Object::ctor" << std::endl;
  }

  ~Object()
  {
    //- std::cout << "Object::dtor" << std::endl;
  }
  
  //-------------------------------------------------
  // Add some bytes to the Object memory footprint
  //-------------------------------------------------
  unsigned char uc[512];
  double db[512];
  
  //-------------------------------------------------
  // ObjectAllocator
  //-------------------------------------------------
  static yat::CachedAllocator<Object> * cache;
  
  //-------------------------------------------------
  // Instances Counter
  //-------------------------------------------------
  static size_t inst_cnt;
};



