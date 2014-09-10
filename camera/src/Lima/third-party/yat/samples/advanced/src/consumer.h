/*!
 * \file     
 * \brief    An advanced yat example
 * \author   N. Leclercq - Synchrotron SOLEIL
 */

#pragma once

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <iostream>
#include <yat/threading/Task.h>

// ============================================================================
// class: Consumer
// ============================================================================
class Consumer: public yat::Task
{
public:

	//- ctor ---------------------------------
	Consumer ();

	//- dtor ---------------------------------
	virtual ~Consumer (void);

protected:
	//- handle_message -----------------------
	virtual void handle_message (yat::Message& msg)
		throw (yat::Exception);

private:
  size_t index;
  
  void parse_objects (size_t no)
    throw (yat::Exception);
};
