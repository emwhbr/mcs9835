/***********************************************************************
*                                                                      *
* Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* BACH - BAsic CHar module                                             *
*                                                                      *
* Usage:                                                               *
* 1. Create device-file: mknod /dev/bach0 c MAJOR 0                    *
*    The major number (MAJOR) will be written by this module OR        *
*    can be checked by looking at the file /proc/devices.              *
*                                                                      *
* 2. READ-operation                                                    *
*    dd if=/dev/bach0 of=/dev/null bs=55 count=2                       *
*                                                                      *
* 3. WRITE-operation:                                                  *
*    dd if=/dev/zero of=/dev/bach0 bs=22 count=5                       *
*                                                                      *
************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>

/* The global variables */
static int bach_major;
static int bach_minor   = 0;
static int bach_nr_devs = 1;

static struct bach_dev *bach_devices = NULL; /* Allocated in 'bach_init' */

/* Function prototypes */
static void bach_exit(void);

/* The BACH-device */
struct bach_dev {
  int id;           /* Identifier            */
  int cnt;          /* User counter          */
  struct cdev cdev; /* Char device structure */
};

/*****************************************************************************
 * Read
 *****************************************************************************/
static ssize_t bach_read(struct file *filp, char __user *buf,
			 size_t count,
			 loff_t *f_pos)
{
  struct bach_dev *dev = filp->private_data; 

  printk(KERN_ALERT "BACH: bach_read, ID=%d, CNT=%d, count=%d\n",
	 dev->id, dev->cnt, (int)count);

  return count;
}

/*****************************************************************************
 * Write
 *****************************************************************************/
static ssize_t bach_write(struct file *filp,
			  const char __user *buf,
			  size_t count,
			  loff_t *f_pos)
{
  struct bach_dev *dev = filp->private_data;

  printk(KERN_ALERT "BACH: bach_write, ID=%d, CNT=%d, count=%d\n",
	 dev->id, dev->cnt, (int)count);

  return count;
}

/*****************************************************************************
 * Open
 *****************************************************************************/
static int bach_open(struct inode *inode,
		     struct file *filp)
{
  struct bach_dev *dev;

  /* Save device information for later usage */
  dev = container_of(inode->i_cdev, struct bach_dev, cdev);
  dev->cnt++;
  filp->private_data = dev;

  printk(KERN_ALERT "BACH: bach_open, ID=%d, CNT=%d\n",
	 dev->id, dev->cnt);

  return 0;
}

/*****************************************************************************
 * Release
 *****************************************************************************/
static int bach_release(struct inode *inode,
			struct file *filp)
{
  struct bach_dev *dev = filp->private_data;
  dev->cnt--;
  printk(KERN_ALERT "BACH: bach_release, ID=%d, CNT=%d\n",
	 dev->id, dev->cnt);

  return 0;
}

/* The file operations */
struct file_operations bach_fops = {
  .owner   = THIS_MODULE,
  .read    = bach_read,
  .write   = bach_write,
  .open    = bach_open,
  .release = bach_release,
};

/*****************************************************************************
 * Setup the char_dev structure for this device.
 *****************************************************************************/
static void bach_setup_cdev(struct bach_dev *dev,
			    int index)
{
  int rc;
  dev_t devno = MKDEV(bach_major, bach_minor + index);
  
  cdev_init(&dev->cdev, &bach_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops   = &bach_fops;
  rc = cdev_add(&dev->cdev, devno, 1);

  /* Fail gracefully if need be */
  if (rc) {
    printk(KERN_ALERT "BACH: Error %d adding bach%d",
	   rc, index);
  }
}

/*****************************************************************************
 * Initialize the module.
 *****************************************************************************/
static int bach_init(void)
{
  int rc, i;
  dev_t dev;
  
  printk(KERN_ALERT "BACH: bach_init\n");

  /* Allocate the device numbers - Dynamically */
  rc = alloc_chrdev_region(&dev, bach_minor, bach_nr_devs, "bach");
  bach_major = MAJOR(dev);
  if (rc < 0) {
    printk(KERN_ALERT "BACH: Can't get major %d\n", bach_major);
    return rc;
  }
  printk(KERN_ALERT "BACH: Device major:%d, minor:%d\n",
	 bach_major, bach_minor);

  /* Allocate the devices */
  bach_devices = kmalloc(bach_nr_devs * sizeof(struct bach_dev), GFP_KERNEL);
  if (!bach_devices) {
    rc = -ENOMEM;
    goto fail;
  }
  memset(bach_devices, 0, bach_nr_devs * sizeof(struct bach_dev));

  /* Initialize each device */
  for (i = 0; i < bach_nr_devs; i++) {
    bach_devices[i].id  = i + 100;
    bach_devices[i].cnt = 0;
    bach_setup_cdev(&bach_devices[i], i);
  }

  /* Initialization OK */
  return 0;

  /* Initialization FAILED */
fail:
  bach_exit();
  return rc;
}

/*****************************************************************************
 * Finalize the module.
 *****************************************************************************/
static void bach_exit(void)
{
  int i;
  dev_t dev = MKDEV(bach_major, bach_minor);

  printk(KERN_ALERT "BACH: bach_exit\n");

  /* Get rid of our char dev entries */
  if (bach_devices) {
    printk(KERN_ALERT "BACH: freeing all devices\n");
    for (i = 0; i < bach_nr_devs; i++) {
      printk(KERN_ALERT "BACH: freeing device:%d\n", i);
      cdev_del(&bach_devices[i].cdev);
    }
    kfree(bach_devices);
  }

  /* 'bach_exit' is never called if registering failed */
  unregister_chrdev_region(dev, bach_nr_devs);
}

module_init(bach_init);
module_exit(bach_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Håkan Brolin");
MODULE_VERSION("R1A01");
MODULE_DESCRIPTION("BACH - BAsic CHar module");
