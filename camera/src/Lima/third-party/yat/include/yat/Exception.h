//----------------------------------------------------------------------------
// YAT LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (C) 2006-2012  N.Leclercq & The Tango Community
//
// Part of the code comes from the ACE Framework (asm bytes swaping code)
// see http://www.cs.wustl.edu/~schmidt/ACE.html for more about ACE
//
// The thread native implementation has been initially inspired by omniThread
// - the threading support library that comes with omniORB. 
// see http://omniorb.sourceforge.net/ for more about omniORB.
//
// Contributors form the TANGO community:
// See AUTHORS file 
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
 * \author See AUTHORS file
 */

#ifndef _YAT_EXCEPTION_H_
#define _YAT_EXCEPTION_H_

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <string>
#include <vector>
#include <yat/CommonHeader.h>

namespace yat 
{

#define THROW_YAT_ERROR( reason, desc, origin ) \
  throw yat::Exception( (reason), (desc), (origin) )

#define RETHROW_YAT_ERROR( exception, reason, desc, origin ) \
  { \
    exception.push_error( (reason), (desc), (origin) ); \
    throw exception; \
  }

  // ============================================================================
  //! \brief Error severity.
  // ============================================================================
  typedef enum {
    //! Warning.
    WARN, 
    //! Functional error.
    ERR, 
    //! Fatal error.
    PANIC
  } ErrorSeverity;

  // ============================================================================
  //! \class Error 
  //! \brief Yat error class.
  //!
  //! The Yat error class provides an implementation of an application error. 
  //! This application error is defined with the following features:
  //! - reason: gives the error type. The goal of this text is to guide the 
  //! \b operator towards the functional cause of the problem.
  //!
  //! - description: describes the error. The goal of this text is to guide the
  //! <b>system expert</b> towards the cause of the problem.
  //!
  //! - origin: gives the function name where the error occurred. The goal of this
  //! text is to guide the \b developper towards the programming error.
  //!
  //! - code: gives the error code. The goal of this number is to identify the error.
  //! - severity: gives the error severity (from yat::ErrorSeverity).
  // ============================================================================
  struct YAT_DECL Error
  {
    //! \brief Default constructor.
    //!
    //! The default values are :
    //! - reason = "unknown"
    //! - desc = "unknown error"
    //! - origin = "unknown"
    //! - code = -1
    //! - severity = yat::ERR
    Error ();

    //! \brief Constructor with parameters.
    //! \param reason %Error reason.
    //! \param desc %Error description.
    //! \param origin %Error origin.
    //! \param err_code %Error id.
    //! \param severity %Error severity.
    Error ( const char *reason,
            const char *desc,
            const char *origin,
            int err_code = -1, 
            int severity = yat::ERR);

    //! \brief Constructor with parameters.
    //! \param reason %Error reason.
    //! \param desc %Error description.
    //! \param origin %Error origin.
    //! \param err_code %Error id.
    //! \param severity %Error severity.    
    Error ( const std::string& reason,
            const std::string& desc,
            const std::string& origin, 
            int err_code = -1, 
            int severity = yat::ERR);

    //! \brief Copy constructor. 
    //! \param src The source error.
    Error (const Error& src);

    //!\brief Destructor.
    virtual ~Error ();

    //! \brief operator=.
    //! \param _src The source error.
    Error& operator= (const Error& _src);

    //! \brief %Error details: reason.
    std::string reason;

    //! \brief %Error details: description.
    std::string desc;

    //! \brief %Error details: origin.
    std::string origin;

    //! \brief %Error details: code.
    int code;

    //! \brief %Error details: severity.
    int severity;
  };

  // ============================================================================
  //! \class Exception 
  //! \brief Yat exception class.
  //!
  //! The Yat exception class provides an implementation of an application exception. 
  //! This application exception contains a list of yat::Error errors.
  // ============================================================================
  class YAT_DECL Exception
  {
  public:

    //! \brief List of errors.
    typedef std::vector<Error> ErrorList;

    //! \brief Default constructor.
    Exception ();

    //! \brief Constructor from an application error.
    //! \param reason %Error reason.
    //! \param desc %Error description.
    //! \param origin %Error origin.
    //! \param err_code %Error code.
    //! \param severity %Error severity.
    Exception ( const char *reason,
                const char *desc,
                const char *origin,
                int err_code = -1, 
                int severity = yat::ERR);

    //! \brief Constructor from an application error.
    //! \param reason %Error reason.
    //! \param desc %Error description.
    //! \param origin %Error origin.
    //! \param err_code %Error code.
    //! \param severity %Error severity.
    Exception ( const std::string& reason,
                const std::string& desc,
                const std::string& origin, 
                int err_code = -1, 
                int severity = yat::ERR);

    //! \brief Constructor from an application error.
    //! \param error Error.
    Exception (const Error& error);

    //! \brief Copy constructor.
    //! \param src The source exception.
    Exception (const Exception& src);

    //! \brief operator=.
    //! \param _src The source exception.
    Exception& operator= (const Exception& _src); 

    //! \brief Destructor.
    virtual ~Exception ();

    //! \brief Pushes the specified error into the error list.
    //! \param reason %Error reason.
    //! \param desc %Error description.
    //! \param origin %Error origin.
    //! \param err_code %Error code.
    //! \param severity %Error severity.    
    void push_error ( const char *reason,
                      const char *desc,
                      const char *origin, 
                      int err_code = -1, 
                      int severity = yat::ERR);

    //! \brief Pushes the specified error into the error list.
    //! \param reason %Error reason.
    //! \param desc %Error description.
    //! \param origin %Error origin.
    //! \param err_code %Error code.
    //! \param severity %Error severity.    
    void push_error ( const std::string& reason,
                      const std::string& desc,
                      const std::string& origin, 
                      int err_code = -1, 
                      int severity = yat::ERR);

    //! \brief Pushes the specified error into the error list.
    //! \param error Error.
    void push_error (const Error& error);
    
    //! \brief Dumps the content of the exception's error list towards standard output.
    virtual void dump () const;

    //! \brief The error list.
    ErrorList errors;
  };

} // namespace

/*
#if defined (YAT_INLINE_IMPL)
# include <yat/Exception.i>
#endif // YAT_INLINE_IMPL
*/

#endif // _EXCEPTION_H_

