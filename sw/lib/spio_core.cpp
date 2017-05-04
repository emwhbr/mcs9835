// ************************************************************************
// *                                                                      *
// * Copyright (C) 2017 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sstream>
#include <iomanip>

#include "spio_core.h"
#include "spio_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define PRODUCT_NUMBER   "SPIO"
#define RSTATE           "R1A01"

#define ERROR_MUTEX_LOCK(mutex)				\
  ( __extension__ ( { if (pthread_mutex_lock(&mutex)) {	\
	return SPIO_ERROR_MUTEX_FAILURE;		\
      } }))

#define ERROR_MUTEX_UNLOCK(mutex)			  \
  ( __extension__ ( { if (pthread_mutex_unlock(&mutex)) { \
	return SPIO_ERROR_MUTEX_FAILURE;		  \
      } }))

#ifdef DEBUG_PRINTS
// 
// Notes!
// Macro 'debug_printf' can be used anywhere in LIBSPIO.
// The other macros can only be used in function 'update_error'.
// 
#define debug_printf(fmt, args...)  printf("LIBSPIO - "); \
                                    printf(fmt, ##args); \
				    fflush(stdout)

#define debug_linux_error()         printf("LIBSPIO LINUX ERROR - "); \
                                    error(0, errno, NULL); \
				    fflush(stdout)

#define debug_internal_error()      printf("LIBSPIO INTERNAL ERROR\n"); \
				    fflush(stdout)
#else
#define debug_printf(fmt, args...) 
#define debug_linux_error()
#define debug_internal_error()
#endif // DEBUG_PRINTS

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

spio_core::spio_core(void)
{
  m_error_source    = SPIO_INTERNAL_ERROR;
  m_error_code      = 0;
  m_last_error_read = true;
  pthread_mutex_init(&m_error_mutex, NULL); // Use default mutex attributes

  m_initialized = false;
  pthread_mutex_init(&m_init_mutex, NULL); // Use default mutex attributes
}

////////////////////////////////////////////////////////////////

spio_core::~spio_core(void)
{
  pthread_mutex_destroy(&m_error_mutex);
  pthread_mutex_destroy(&m_init_mutex);
}

////////////////////////////////////////////////////////////////

long spio_core::get_last_error(SPIO_LIB_STATUS *status)
{
  try {
    ERROR_MUTEX_LOCK(m_error_mutex);
    status->error_source = m_error_source;
    status->error_code   = m_error_code;
    
    // Clear internal error information
    m_error_source    = SPIO_INTERNAL_ERROR;
    m_error_code      = SPIO_NO_ERROR;
    m_last_error_read = true;
    ERROR_MUTEX_UNLOCK(m_error_mutex);
    return SPIO_SUCCESS;
  }
  catch (...) {
    return set_error(EXP(SPIO_INTERNAL_ERROR, SPIO_UNEXPECTED_EXCEPTION, NULL, NULL));
  }
}

////////////////////////////////////////////////////////////////

long spio_core::get_error_string(long error_code, 
				 SPIO_ERROR_STRING error_string)
{
  try {
    // Check input values
    if (!error_string) {
      THROW_EXP(SPIO_INTERNAL_ERROR, SPIO_BAD_ARGUMENT,
		"error_string is null pointer", NULL);
    }

    // Do the actual work
    return internal_get_error_string(error_code, error_string);
  }
  catch (spio_exception &sxp) {
    return set_error(sxp);
  }
  catch (...) {
    return set_error(EXP(SPIO_INTERNAL_ERROR, SPIO_UNEXPECTED_EXCEPTION, NULL, NULL));
  }
}

////////////////////////////////////////////////////////////////

long spio_core::initialize(void)
{
  try {
    if (spio_do_mutex_lock(&m_init_mutex) != SPIO_SUCCESS) {
      THROW_EXP(SPIO_LINUX_ERROR, SPIO_MUTEX_LOCK_FAILED,
		"Mutex lock failed", NULL);
    }

    // Check if already initialized
    if (m_initialized) {
       THROW_EXP(SPIO_INTERNAL_ERROR, SPIO_ALREADY_INITIALIZED,
		 "Already initialized", NULL);
    }

    // Do the actual initialization
    internal_initialize();

    // Initialization completed    
    m_initialized = true;

    if (spio_do_mutex_unlock(&m_init_mutex) != SPIO_SUCCESS) {
      THROW_EXP(SPIO_LINUX_ERROR, SPIO_MUTEX_UNLOCK_FAILED,
		"Mutex unlock failed", NULL);
    }

    return SPIO_SUCCESS;
  }
  catch (spio_exception &sxp) {
    spio_do_mutex_unlock(&m_init_mutex);
    return set_error(sxp);
  }
  catch (...) {
    spio_do_mutex_unlock(&m_init_mutex);
    return set_error(EXP(SPIO_INTERNAL_ERROR, SPIO_UNEXPECTED_EXCEPTION, NULL, NULL));
  }
}

////////////////////////////////////////////////////////////////

long spio_core::finalize(void)
{
  try {
    if (spio_do_mutex_lock(&m_init_mutex) != SPIO_SUCCESS) {
      THROW_EXP(SPIO_LINUX_ERROR, SPIO_MUTEX_LOCK_FAILED,
		"Mutex lock failed", NULL);
    }

    // Check if not initialized
    if (!m_initialized) {
      THROW_EXP(SPIO_INTERNAL_ERROR, SPIO_NOT_INITIALIZED,
		"Not initialized", NULL);
    }   

    // Do the actual finalization
    internal_finalize();

    // Finalization completed   
    m_initialized = false;

    if (spio_do_mutex_unlock(&m_init_mutex) != SPIO_SUCCESS) {
      THROW_EXP(SPIO_LINUX_ERROR, SPIO_MUTEX_UNLOCK_FAILED,
		"Mutex unlock failed", NULL);
    }

    return SPIO_SUCCESS;
  }
  catch (spio_exception &sxp) {
    spio_do_mutex_unlock(&m_init_mutex);
    return set_error(sxp);
  }
  catch (...) {
    spio_do_mutex_unlock(&m_init_mutex);
    return set_error(EXP(SPIO_INTERNAL_ERROR, SPIO_UNEXPECTED_EXCEPTION, NULL, NULL));
  }
}

////////////////////////////////////////////////////////////////

long spio_core::test_get_lib_prod_info(SPIO_LIB_PROD_INFO *prod_info)
{
  try {
    // Check input values
    if (!prod_info) {
      THROW_EXP(SPIO_INTERNAL_ERROR, SPIO_BAD_ARGUMENT,
		"prod_info is null pointer", NULL);
    }

    // Do the actual work
    return internal_test_get_lib_prod_info(prod_info);
  }
  catch (spio_exception &sxp) {
    return set_error(sxp);
  }
  catch (...) {
    return set_error(EXP(SPIO_INTERNAL_ERROR, SPIO_UNEXPECTED_EXCEPTION, NULL, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

long spio_core::set_error(spio_exception sxp)
{
#ifdef DEBUG_PRINTS

  // Get the stack trace
  STACK_FRAMES frames;
  sxp.get_stack_frames(frames);

  ostringstream oss_msg;
  char buffer[18];

  oss_msg << "stack frames:" << (int) frames.active_frames << "\n";

  for (unsigned i=0; i < frames.active_frames; i++) {
    sprintf(buffer, "0x%08x", frames.frames[i]);
    oss_msg << "\tframe:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\n";
  }

  // Get identification
  oss_msg << "\tPid: " << sxp.get_process_id() 
	  << ", Tid: " << sxp.get_thread_id() << "\n";

  // Get info from predefined macros
  oss_msg << "\tVspiolator: " << sxp.get_file() 
	  << ":" << sxp.get_line()
	  << ", " << sxp.get_function() << "\n";

  // Get the internal info
  oss_msg << "\tSource: " << sxp.get_source()
	  << ", Code: " << sxp.get_code() << "\n";

  oss_msg << "\tInfo: " << sxp.get_info() << "\n";

  // Print all info
  debug_printf(oss_msg.str().c_str(), NULL);

#endif // DEBUG_PRINTS

  // Update internal error information
  return update_error(sxp);
}

////////////////////////////////////////////////////////////////

long spio_core::update_error(spio_exception sxp)
{
  ERROR_MUTEX_LOCK(m_error_mutex);
  if (m_last_error_read) {
    m_error_source    = sxp.get_source();
    m_error_code      = sxp.get_code();
    m_last_error_read = false; /* Latch last error until read */
  }
  ERROR_MUTEX_UNLOCK(m_error_mutex);

#ifdef DEBUG_PRINTS 
  switch(sxp.get_source()) {
  case SPIO_INTERNAL_ERROR:
    debug_internal_error();
    break;
  case SPIO_LINUX_ERROR:
    debug_linux_error();
    break;
  }
#endif

  return SPIO_FAILURE;
}

////////////////////////////////////////////////////////////////

long spio_core::internal_get_error_string(long error_code,
					  SPIO_ERROR_STRING error_string)
{
  size_t str_len = sizeof(SPIO_ERROR_STRING);

  switch (error_code) {
  case SPIO_NO_ERROR:
    strncpy(error_string, "No error", str_len);
    break;
  case SPIO_NOT_INITIALIZED:
    strncpy(error_string, "Not initialized", str_len);
    break;
  case SPIO_ALREADY_INITIALIZED:
    strncpy(error_string, "Already initialized", str_len);
    break;
  case SPIO_BAD_ARGUMENT:
    strncpy(error_string, "Bad argument", str_len);
    break;
  case SPIO_MUTEX_LOCK_FAILED:
    strncpy(error_string, "Mutex lock failed", str_len);
    break;
  case SPIO_MUTEX_UNLOCK_FAILED:
    strncpy(error_string, "Mutex unlock failed", str_len);
    break;
  case SPIO_UNEXPECTED_EXCEPTION:
    strncpy(error_string, "Unexpected exception", str_len);
    break;
  default: 
    strncpy(error_string, "Undefined error", str_len);
  }

  return SPIO_SUCCESS;
}

////////////////////////////////////////////////////////////////

void spio_core::internal_initialize(void)
{
  // Create the TBD object with garbage collector
}

////////////////////////////////////////////////////////////////

void spio_core::internal_finalize(void)
{
  SPIO_LIB_STATUS status; 
  get_last_error(&status);  
}

////////////////////////////////////////////////////////////////

long spio_core::internal_test_get_lib_prod_info(SPIO_LIB_PROD_INFO *prod_info)
{
  strncpy(prod_info->prod_num, 
	  PRODUCT_NUMBER, 
	  sizeof(((SPIO_LIB_PROD_INFO *)0)->prod_num));

  strncpy(prod_info->rstate, 
	  RSTATE, 
	  sizeof(((SPIO_LIB_PROD_INFO *)0)->rstate));

  return SPIO_SUCCESS;
}
