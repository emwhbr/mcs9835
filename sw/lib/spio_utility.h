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

#ifndef __SPIO_UTILITY_H__
#define __SPIO_UTILITY_H__

#include <pthread.h>

using namespace std;

extern long spio_do_mutex_lock(pthread_mutex_t *mutex);

extern long spio_do_mutex_unlock(pthread_mutex_t *mutex);

extern long spio_get_my_pid(void);

extern long spio_get_my_thread_id(void);

extern long spio_do_nanosleep(double timesec);

#endif // __SPIO_UTILITY_H__
