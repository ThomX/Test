/*!
 * \file     
 * \brief    An advanced yat example
 * \author   N. Leclercq - Synchrotron SOLEIL
 */

#include <iostream>
#include <yat/threading/Thread.h>
#include "context.h"
#include "producer.h"
#include "consumer.h"

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char* argv[])
{
  //- initialize the context
  Context::init(10, 10000);
  
  //- num of consumers to instanciate
  size_t nc = 2;
  
  //- instanciate/start the objects consumers 
  for (size_t i = 0; i < nc; i++)
  {
    Consumer * c = new Consumer ();
    Context::instance().consumers.push_back(c);
    c->go();
  }
  
  //- instanciate/start the objects producer 
  Producer * p = new Producer (); 
  p->go();
  
  //- give producer/consumers some time to run
  yat::Thread::sleep(10000);
  
  //- ask consumers to reset their parsing index
  for (size_t i = 0; i < nc; i++)
    Context::instance().consumers[i]->post(RESET_INDEX_MSG);
  
  //- give producer/consumers some time to run
  yat::Thread::sleep(20000);
  
  //- ask producer to exit
  p->exit();
  
  //- ask consumers to exit
  for (size_t i = 0; i < nc; i++)
    Context::instance().consumers[i]->exit();
  
  //- release the context
  Context::fini();
  
  return 0;
}
