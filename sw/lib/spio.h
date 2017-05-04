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

#ifndef __SPIO_H__
#define __SPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LIBSPIO return codes
 */
#define SPIO_SUCCESS               0
#define SPIO_FAILURE              -1
#define SPIO_ERROR_MUTEX_FAILURE  -2

/*
 * LIBSPIO Internal error codes
 */
#define SPIO_NO_ERROR                      0
#define SPIO_NOT_INITIALIZED               1
#define SPIO_ALREADY_INITIALIZED           2
#define SPIO_BAD_ARGUMENT                  3
#define SPIO_MUTEX_LOCK_FAILED             4
#define SPIO_MUTEX_UNLOCK_FAILED           5
#define SPIO_UNEXPECTED_EXCEPTION          6

/*
 * Error source values
 */
typedef enum {SPIO_INTERNAL_ERROR, 
	      SPIO_LINUX_ERROR} SPIO_ERROR_SOURCE;
/*
 * Basic API support types
 */

/*
 * API types
 */
typedef char SPIO_ERROR_STRING[256];

typedef struct {
  char prod_num[20];
  char rstate[10];
} SPIO_LIB_PROD_INFO;

typedef struct {
  SPIO_ERROR_SOURCE error_source;
  long            error_code;
} SPIO_LIB_STATUS;

/****************************************************************************
*
* Name spio_get_last_error
*
* Description Returns the error information held by LIBSPIO, when a LIBSPIO 
*             call returns unsuccessful completion. 
*             LIBSPIO clears its internal error information after it has been 
*             read by the calling application.
*
* Parameters status  IN/OUT  pointer to a buffer to hold the error information
*
* Error handling Returns SPIO_SUCCESS if successful
*                otherwise SPIO_FAILURE or SPIO_ERROR_MUTEX_FAILURE
*
****************************************************************************/
 extern long spio_get_last_error(SPIO_LIB_STATUS *status);

/****************************************************************************
*
* Name spio_get_error_string
*
* Description Returns the error string corresponding to the provided
*             internal error code.
*
* Parameters error_code    IN      actual error code
*            error_string  IN/OUT  pointer to a buffer to hold the error string
*
* Error handling Returns always SPIO_SUCCESS
*
****************************************************************************/
extern long spio_get_error_string(long error_code, 
				  SPIO_ERROR_STRING error_string);

/****************************************************************************
*
* Name spio_initialize
*
* Description Allocates necessary system resources.
*             This function shall be called once to make LIBSPIO operational.
*             This function can be called again after finalization.
*
* Parameters None
*
* Error handling Returns SPIO_SUCCESS if successful
*                otherwise SPIO_FAILURE or SPIO_ERROR_MUTEX_FAILURE
*
****************************************************************************/
extern long spio_initialize(void);

/****************************************************************************
*
* Name spio_finalize
*
* Description Deallocates system resources created during initialization.
*
* Parameters None 
*
* Error handling Returns SPIO_SUCCESS if successful
*                otherwise SPIO_FAILURE or SPIO_ERROR_MUTEX_FAILURE
*
****************************************************************************/
extern long spio_finalize(void);

/****************************************************************************
*
* Name spio_test_get_lib_prod_info
*
* Description Returns the product- and revision information of LIBSPIO.
*
* Parameters prod_info  IN/OUT  pointer to a buffer to hold the product number
*                               and the RState.
*
* Error handling Returns always SPIO_SUCCESS.
*
****************************************************************************/
extern long spio_test_get_lib_prod_info(SPIO_LIB_PROD_INFO *prod_info);

#ifdef  __cplusplus
}
#endif

#endif /* __SPIO_H__ */
