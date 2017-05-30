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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>

#include "mcs9835_cdev.h"
#include "mcs9835_log.h"

/****************************************************************************
 *
 * Exported functions
 *
 ****************************************************************************/

/****************************************************************************/

int mcs9835_cdev_initialize(struct mcs9835_dev *dev,
			    int dev_idx)
{
  int rc = 0;

  char class_name[50];

  /* 
   * Create device class.
   * Listed in sysfs according to: /sys/class/mcs9835_c<dev_idx>
   */
  sprintf(class_name, "%s_c%d", DRV_NAME, dev_idx);

  LOG(MCS_CDV, "create device class %s\n", class_name);
  dev->class = class_create(THIS_MODULE, class_name);
  if (IS_ERR(dev->class)) {
    LOG(MCS_ERR, "class_create failed for %s\n", class_name);
    rc = PTR_ERR(dev->class);
  }

  return rc;
}

/****************************************************************************/

void mcs9835_cdev_finalize(struct mcs9835_dev *dev)
{
  /* 
   * Remove device class.
   */
  if (dev->class) {
    LOG(MCS_CDV, "destroy device class\n");
    class_destroy(dev->class);    
    dev->class = NULL;
  }
}

/****************************************************************************/

int mcs9835_cdev_create(struct mcs9835_dev *dev,
			int dev_idx,
			int cdev_idx, 
			struct file_operations *fops)
{
  int rc = -1;
  
  int major  = 0;
  int minor  = 0;

  const int count = 1;

  struct device *device = NULL;

  if (dev->class == NULL) {
    LOG(MCS_ERR, "CDEV class must be initialized before adding devices\n");
    goto cdev_err_alloc_region;
  }

  /* 
   * Allocate cdev region. 
   */
  LOG(MCS_CDV, "alloc_chrdev_region, idx:%d\n", cdev_idx);
  rc = alloc_chrdev_region(&(dev->chr[cdev_idx].cdevno), 
			   minor,
			   count,
			   DRV_NAME);
  if (rc) {
    LOG(MCS_ERR, "alloc_chrdev_region failed, idx:%d\n", cdev_idx);
    goto cdev_err_alloc_region;
  }

  /* Fetch allocated MAJOR number */
  major = MAJOR(dev->chr[cdev_idx].cdevno); 

  /* 
   * Initialize cdev.
   */
  LOG(MCS_CDV, "cdev_init\n");
  cdev_init(&dev->chr[cdev_idx].cdev,
	    fops);
  dev->chr[cdev_idx].cdev.owner = THIS_MODULE;

  /* 
   * Add cdev.
   */
  LOG(MCS_CDV, "cdev_add\n");
  rc = cdev_add(&dev->chr[cdev_idx].cdev, 
		dev->chr[cdev_idx].cdevno, 
		count);
  if (rc < 0) {
    LOG(MCS_ERR, "cdev_add failed, idx:%d\n", cdev_idx);
    goto cdev_err_add;
  }

  /* 
   * Create device file.
   * Listed accroding to: /dev/mcs9835_<dev_idx>_<cdev_idx>
   */
  LOG(MCS_CDV, "device_create, %s_%d_%d\n",
      DRV_NAME, dev_idx, cdev_idx);
  device = device_create(dev->class,
			 NULL,
			 MKDEV(major, minor),
			 NULL, 
			 DRV_NAME "_%d_%d", dev_idx, cdev_idx);
  if (IS_ERR(device)) {
    rc = PTR_ERR(device);
    LOG(MCS_ERR, "Failed to create device file %s_%d_%d\n",
	DRV_NAME, dev_idx, cdev_idx);
    goto cdev_err_create;
  }
  
  /* 
   * All ok, indicate cdev was initialized.
  */
  dev->chr[cdev_idx].have_cdev = 1;

  return 0;

  /***************
   * Error cases 
   ***************/
 cdev_err_create:
  cdev_del(&(dev->chr[cdev_idx].cdev));

 cdev_err_add:
  unregister_chrdev_region(dev->chr[cdev_idx].cdevno,
			   count);

 cdev_err_alloc_region:
  
  /* 
   * Indicate cdev not initialized.
   */
  dev->chr[cdev_idx].have_cdev = 0;
  
  return rc;
}

/****************************************************************************/

void mcs9835_cdev_destroy(struct mcs9835_dev *dev,
			  int cdev_idx)
{
  int major  = MAJOR(dev->chr[cdev_idx].cdevno);
  int minor  = 0;

  const int count = 1;

  if (dev) {
    /* 
     * Remove device node file. 
     */
    if (dev->class && dev->chr[cdev_idx].have_cdev) {
      LOG(MCS_CDV, "device_destroy, idx:%d\n", cdev_idx); 
      device_destroy(dev->class, MKDEV(major, minor));      
    }
    
    /* 
     * Remove cdev.
     */
    if (dev->chr[cdev_idx].have_cdev) {
      LOG(MCS_CDV, "cdev_del\n");
      cdev_del(&(dev->chr[cdev_idx].cdev));  
    }

    /* 
     * Unregister cdev.
     */
    if (dev->chr[cdev_idx].have_cdev) {
      LOG(MCS_CDV, "unregister_cdev_region\n");
      unregister_chrdev_region(dev->chr[cdev_idx].cdevno, count);  
    }
    dev->chr[cdev_idx].have_cdev = 0;

  } else {
    /* 
     * No cdev allocated - nothing to release.
    */
    LOG(MCS_WRN, "Private data is NULL, cdev cleanup not possible\n");
  }
}
