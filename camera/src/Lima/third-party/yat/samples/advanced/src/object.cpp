/*!
 * \file     
 * \brief    An advanced yat example
 * \author   N. Leclercq - Synchrotron SOLEIL
 */

// ============================================================================
// Dependencies
// ============================================================================
#include "object.h"

// ============================================================================
// Object static members
// ============================================================================
size_t Object::inst_cnt = 0;
yat::CachedAllocator<Object> * Object::cache = 0;


