/*!
 * \file     
 * \brief    An advanced yat example
 * \author   N. Leclercq - Synchrotron SOLEIL
 */

#pragma once

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <deque>
#include <yat/threading/Task.h>
#include "object.h"

// ============================================================================
// forward declarations
// ============================================================================
class Consumer;

// ============================================================================
// forward declarations
// ============================================================================
#define NEW_DATA_AVAILABLE_MSG      10000
#define RESET_INDEX_MSG             10001
  
// ============================================================================
// typedefs
// ============================================================================
typedef std::deque<Object*> ObjectsRepository;
typedef std::vector<yat::Task*> ConsumersRepository;

// ============================================================================
// struct: Context
// ============================================================================
struct Context
{   
  static void init (size_t nb = 10, size_t bs = 1000)
  {
    //- initialize the object memory cache
    Object::init(nb, bs);
    //- instanciate the context singleton
    Context::singleton = new Context;
  }
  
  static void fini ()
  {
    //- release instanciated objects
    size_t i = 0;
    size_t n = Context::singleton->data.size();
    while (i < n)
      Object::release(Context::singleton->data[i++]);
    //- release the object memory cache
    Object::fini();
    //- release the context singleton
    delete Context::singleton;
    Context::singleton = 0;
  }
  
  inline static Context & instance ()
  {
    return * Context::singleton;
  }
  
  //- the consumers repository 
  ConsumersRepository  consumers;
 
  //- the objects repository 
  ObjectsRepository  data;
  
  //- the Object repository 
  static Context * singleton;
};

