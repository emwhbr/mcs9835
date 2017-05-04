/************************************************************************
 *                                                                      *
 * Copyright (C) 2017 Bonden i Nol (hakanbrolin@hotmail.com)            *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "spio.h"

#ifdef DEBUG_PRINTS
#define debug_test_printf(fmt, args...)  printf("DBG - "); printf(fmt, ##args); fflush(stdout)
#else
#define debug_test_printf(fmt, args...) 
#endif /* DEBUG_PRINTS */

/*
 * ---------------------------------
 *       Macros
 * ---------------------------------
 */
#define TEST_LIBSPIO_ERROR_MSG "*** ERROR : test_libspio\n"

/*
 * ---------------------------------
 *       Types
 * ---------------------------------
 */

/*
 * ---------------------------------
 *       Global variables
 * ---------------------------------
 */

/*
 * ---------------------------------
 *       Function prototypes
 * ---------------------------------
 */
static void get_prod_info(void);
static void get_last_error(void);
static void initialize(void);
static void finalize(void);
static void print_menu(void);
static void do_test_libspio(void);

/*****************************************************************/

static void get_prod_info(void)
{
  SPIO_LIB_PROD_INFO prod_info;

  if (spio_test_get_lib_prod_info(&prod_info)) {
    printf(TEST_LIBSPIO_ERROR_MSG);
    return;
  }
  printf("LIBSPIO prod num: %s\n", prod_info.prod_num);
  printf("LIBSPIO rstate  : %s\n", prod_info.rstate);
}

/*****************************************************************/

static void get_last_error(void)
{
  SPIO_LIB_STATUS status;
  SPIO_ERROR_STRING error_string;

  if (spio_get_last_error(&status) != SPIO_SUCCESS) {
    printf(TEST_LIBSPIO_ERROR_MSG);
    return;
  }

  if (spio_get_error_string(status.error_code,
			    error_string) != SPIO_SUCCESS) {
    printf(TEST_LIBSPIO_ERROR_MSG);
    return;
  }

  switch (status.error_source) {
  case SPIO_INTERNAL_ERROR:
    printf("LIBSPIO error source : SPIO_INTERNAL_ERROR\n");
    break;
  case SPIO_LINUX_ERROR:
    printf("LIBSPIO error source : SPIO_LINUX_ERROR\n");
    break;
  default:
    printf("LIBSPIO error source : *** UNKNOWN\n");
  }
  printf("LIBSPIO error code   : %ld\n", status.error_code);
  printf("LIBSPIO error string : %s\n",  error_string);
}

/*****************************************************************/

static void initialize(void)
{
  if (spio_initialize() != SPIO_SUCCESS) {
    printf(TEST_LIBSPIO_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void finalize(void)
{
  if (spio_finalize() != SPIO_SUCCESS) {
    printf(TEST_LIBSPIO_ERROR_MSG);
  }
}
 
/*****************************************************************/

static void print_menu(void)
{
  printf("\n");
  printf("  1. (test) get product info\n");
  printf("  2. get last error + get error string\n");
  printf("  3. initialize\n");
  printf("  4. finalize\n");
  printf("100. Exit\n\n");
}

/*****************************************************************/

static void do_test_libspio(void)
{
  int rc;
  int value;

  do {
    print_menu();
    
    printf("Enter choice : ");
    rc=scanf("%d",&value);
    if (rc != 1) {
      printf("Illegal choice!\n");
      continue;
    }
    
    switch(value) {
    case 1:
      get_prod_info();
      break;
    case 2:
      get_last_error();
      break;
    case 3:
      initialize();
      break;
    case 4:
      finalize();
      break;
    case 100: /* Exit */
      break;
    default:
      printf("Illegal choice!\n");
    }
  } while (value != 100);
}

/*****************************************************************/

int main(void)
{
  do_test_libspio();

  printf("Goodbye!\n");
  return 0;
}
