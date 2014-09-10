/*!
 * \file     
 * \brief    An advanced yat example
 * \author   N. Leclercq - Synchrotron SOLEIL
 */

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <iostream>
#include "context.h"
#include "consumer.h"

// ============================================================================
// Consumer::Consumer
// ============================================================================
Consumer::Consumer ()
{
	YAT_TRACE("Consumer::Consumer");
}

// ======================================================================
// Consumer::~Consumer
// ======================================================================
Consumer::~Consumer (void)
{
	YAT_TRACE("Consumer::~Consumer");
}

// ============================================================================
// Consumer::handle_message
// ============================================================================
void Consumer::handle_message (yat::Message& _msg)
	throw (yat::Exception)
{
	//- YAT_TRACE("Consumer::handle_message");

	//- handle msg
  switch (_msg.type())
	{
	  //- TASK_INIT ----------------------
	  case yat::TASK_INIT:
	    {
  	    //- "initialization" code goes here
  	    this->index = 0;
        std::cout << "Consumer::"
                  << this
                  << "::init"
                  << std::endl;
      }
		  break;
		//- TASK_EXIT ----------------------
		case yat::TASK_EXIT:
		  {
        std::cout << "Consumer::"
                  << this
                  << "::exit::parsed " 
                  << this->index
                  << " objs" 
                  << std::endl;
      }
			break;
    //- NEW_DATA_AVAILABLE_MSG
    case NEW_DATA_AVAILABLE_MSG:
      {
        //- get data associated with the message 
        size_t no = _msg.get_data<size_t>();
        //- parse last <no> objects in repository
        this->parse_objects( no );
      }
      break;
    //- DATA_REPOSITORY_RESETED_MSG
    case RESET_INDEX_MSG:
      {
        //- reset index
        this->index = 0;
        //- parse all objects in repository
        this->parse_objects( Context::instance().data.size() );
      }
      break;
  	default:
  		break;
	}
}

// ============================================================================
// Consumer::parse_objects
// ============================================================================
void Consumer::parse_objects (size_t no)
  throw (yat::Exception)
{
  //- parse new objects
  const size_t max = this->index + no;
  
  /*
  std::cout << "Consumer::"
            << this
            << "::parse_objects::parsing objs {" 
            << Context::instance().data[this->index]->inst_id
            << ","
            << Context::instance().data[max - 1]->inst_id
            << "}" 
            << std::endl;
  */

  for (; this->index < max;)
    Context::instance().data[this->index++];
}
