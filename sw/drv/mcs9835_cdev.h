/***********************************************************************
*                                                                      *
* Copyright (C) 2017 Bonden i Nol (hakanbrolin@hotmail.com)            *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
************************************************************************/

#ifndef __MCS9835_CDEV_H__
#define __MCS9835_CDEV_H__

#include <linux/fs.h>

#include "mcs9835.h"

/****************************************************************************
 * 
 * Exported functions
 *
 ****************************************************************************/

extern int mcs9835_cdev_initialize(struct mcs9835_dev *dev,
				   int dev_idx);

extern void mcs9835_cdev_finalize(struct mcs9835_dev *dev);

extern int mcs9835_cdev_create(struct mcs9835_dev *dev,
			       int dev_idx,
			       int cdev_idx, 
			       struct file_operations *fops);

extern void mcs9835_cdev_destroy(struct mcs9835_dev *dev,
				 int cdev_idx);

#endif /* __MCS9835_CDEV_H__ */
