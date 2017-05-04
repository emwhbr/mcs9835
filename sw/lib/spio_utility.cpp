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

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include "spio_utility.h"
#include "spio.h"

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public functions
/////////////////////////////////////////////////////////////////////////////

long spio_do_mutex_lock(pthread_mutex_t *mutex)
{
  if (pthread_mutex_lock(mutex)) {
    return SPIO_FAILURE;
  }
  return SPIO_SUCCESS;
}

////////////////////////////////////////////////////////////////

long spio_do_mutex_unlock(pthread_mutex_t *mutex)
{
  if (pthread_mutex_unlock(mutex)) {
    return SPIO_FAILURE;
  }
  return SPIO_SUCCESS;
}

////////////////////////////////////////////////////////////////

long spio_get_my_pid(void)
{
  return (long)getpid();
}

////////////////////////////////////////////////////////////////

long spio_get_my_thread_id(void)
{
  return (long)syscall(SYS_gettid);
}

////////////////////////////////////////////////////////////////

long spio_do_nanosleep(double timesec)
{
  int rc;
  struct timespec ts;

  if ( timesec < 0.0 ) {
    return SPIO_FAILURE;
  }

  // Extracts seconds and nanoseconds
  ts.tv_sec  = (time_t) timesec;
  ts.tv_nsec = (long int)( (timesec - (time_t)timesec) * 1000000000.0 );

  // Do nanosleep and protect from being interrupted
  // by any signals (EINTR)
  do
    {
      rc = nanosleep(&ts, &ts);
    }
  while ( (rc != 0) && (errno == EINTR) );

  if ( rc != 0 ) {
    return SPIO_FAILURE;
  }

  return SPIO_SUCCESS;
}
