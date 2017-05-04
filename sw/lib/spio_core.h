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

#ifndef __SPIO_CORE_H__
#define __SPIO_CORE_H__

#include <memory>

#include "spio_exception.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class spio_core {

public:
  spio_core(void);
  ~spio_core(void);

  long get_last_error(SPIO_LIB_STATUS *status);

  long get_error_string(long error_code,
			SPIO_ERROR_STRING error_string);

  long initialize(void);

  long finalize(void);

  long test_get_lib_prod_info(SPIO_LIB_PROD_INFO *prod_info);

private:
  // Error handling information
  SPIO_ERROR_SOURCE m_error_source;
  long              m_error_code;
  bool              m_last_error_read;
  pthread_mutex_t   m_error_mutex;

  // Keep track of initialization
  bool             m_initialized;
  pthread_mutex_t  m_init_mutex;

  // Private member functions
  long set_error(spio_exception sxp);

  long update_error(spio_exception sxp);

  long internal_get_error_string(long error_code,
				 SPIO_ERROR_STRING error_string);

  void internal_initialize(void);

  void internal_finalize(void);

  long internal_test_get_lib_prod_info(SPIO_LIB_PROD_INFO *prod_info);
};

#endif // __SPIO_CORE_H__
