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
#include <yat/threading/Task.h>

// ============================================================================
// SOME USER DEFINED MESSAGES
// ============================================================================


// ============================================================================
// class: Producer
// ============================================================================
class Producer: public yat::Task
{
public:

	//- ctor ---------------------------------
	Producer ();

	//- dtor ---------------------------------
	virtual ~Producer (void);

protected:
	//- handle_message -----------------------
	virtual void handle_message (yat::Message& msg)
		throw (yat::Exception);

private:
};
