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

#include "spio.h"
#include "spio_core.h"

////////////////////////////
// Module global variables 
////////////////////////////
static spio_core g_object;

////////////////////////////////////////////////////////////////

long spio_get_last_error(SPIO_LIB_STATUS *status)
{
  return g_object.get_last_error(status);
}

////////////////////////////////////////////////////////////////

long spio_get_error_string(long error_code,
			   SPIO_ERROR_STRING error_string)
{
  return g_object.get_error_string(error_code, error_string);  
}

////////////////////////////////////////////////////////////////

long spio_initialize(void)
{
  return g_object.initialize();
}

////////////////////////////////////////////////////////////////

long spio_finalize(void)
{
  return g_object.finalize();  
}

////////////////////////////////////////////////////////////////

long spio_test_get_lib_prod_info(SPIO_LIB_PROD_INFO *prod_info)
{
  return g_object.test_get_lib_prod_info(prod_info);
}
