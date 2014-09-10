/*!
 * \file     
 * \brief    An example of yat::SharedPtr usage
 * \author   N. Leclercq, J. Malik - Synchrotron SOLEIL
 */

#include <iostream>
#include <yat/memory/SharedPtr.h>

//-----------------------------------------------------------------------------
// MyObject
//-----------------------------------------------------------------------------
struct MyObject
{
  MyObject ( const std::string& s )
    : some_attribute(s)
  {
  	std::cout << "MyObject::calling ctor for " << some_attribute << std::endl;
  }

  ~MyObject()
  {
    std::cout << "MyObject::calling dtor for " << some_attribute << std::endl;
  }

  std::string some_attribute;
};

//-----------------------------------------------------------------------------
// DUMP MACRO
//-----------------------------------------------------------------------------
#define DUMP( ptr ) \
  std::cout << "SharedPtr " \
  					<< #ptr \
            << " -- points to --> " \
            << (ptr ? ptr->some_attribute : "xxxxxx") \
            << " [its ref. count is " \
            << ptr.use_count() \
            << "]" \
            << std::endl
            
//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  typedef yat::SharedPtr<MyObject> MyObjectPtr;

  MyObjectPtr foo ( new MyObject("foo-sp") );
 
  MyObjectPtr bar ( new MyObject("bar-sp") );

  MyObjectPtr tmp;

  std::cout << std::endl; //-----------------------------------------------------
  
  std::cout << "initial state:" << std::endl;
  DUMP( tmp );
  DUMP( foo );
  DUMP( bar );

  std::cout << std::endl; //-----------------------------------------------------
  
  tmp = foo;
  std::cout << "after 'tmp = foo' :" << std::endl;
  
  DUMP( tmp );
  DUMP( foo );
  DUMP( bar );

  std::cout << std::endl; //-----------------------------------------------------
  
  tmp = bar;
  
  std::cout << "after 'tmp = bar' :" << std::endl;
  
  DUMP( tmp );
  DUMP( foo );
  DUMP( bar );

  std::cout << std::endl; //-----------------------------------------------------
  
  tmp.reset();
  
  std::cout << "after 'tmp.reset()' :" << std::endl;
  
  DUMP( tmp );
  DUMP( foo );
  DUMP( bar );

  std::cout << std::endl; //-----------------------------------------------------
  
  tmp = foo; 
  
  foo.reset( new MyObject("oof-so") );
  
  std::cout << "after 'tmp = foo; foo.reset( new MyObject(\"oof-so\") )' :" << std::endl;
  
  DUMP( tmp );
  DUMP( foo );
  DUMP( bar );

  std::cout << std::endl; //-----------------------------------------------------
  
  foo.reset();

  std::cout << "after 'foo.reset()' :" << std::endl;
  
  DUMP( tmp );
  DUMP( foo );
  DUMP( bar );

  std::cout << std::endl; //-----------------------------------------------------

  return 0;
}
