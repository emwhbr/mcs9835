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
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "mcs9835_product_info.h"
#include "mcs9835_log.h"
#include "mcs9835_hw.h"

/****************************************************************************
 *
 * Kernel module information
 *
 ****************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Håkan Brolin");
MODULE_VERSION(DRV_RSTATE);
MODULE_DESCRIPTION("MCS9835 chipset PCI driver");

/****************************************************************************
 *
 * Kernel module parameters
 *
 ****************************************************************************/
/*
 * loglevel: Overrides the default loglevel.
 *           [OFF] by default
 */
u32 loglevel;
module_param(loglevel, int, 0);
MODULE_PARM_DESC(loglevel, "Bitmask (32bit) enabling loglevels");

/****************************************************************************
 *
 * Function prototypes
 *
 ****************************************************************************/
/* Kernel compatibility */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,4,0)
#define DEVEXIT_MARK __devexit
#define DEVINIT_MARK __devinit
#else 
#define DEVEXIT_MARK 
#define DEVINIT_MARK 
#endif

static int DEVINIT_MARK mcs9835_probe(struct pci_dev *dev, 
				      const struct pci_device_id *id);

static void DEVEXIT_MARK mcs9835_remove(struct pci_dev *dev);

static int __init mcs9835_initialize(void);

static void __exit mcs9835_finalize(void);

/****************************************************************************
 *
 * PCI driver infrastructure
 *
 ****************************************************************************/
/* The PCI device table, listing supported devices */
static const struct pci_device_id mcs9835_pci_ids[] = {
  { PCI_DEVICE(MCS9835_VENDOR_ID, MCS9835_DEVICE_ID) },
  { 0 },
};

MODULE_DEVICE_TABLE(pci, mcs9835_pci_ids);

/* Used when register driver with the PCI core */
static struct pci_driver mcs9835_pci_driver = {
    .name     = DRV_NAME,
    .id_table = mcs9835_pci_ids,
    .probe    = mcs9835_probe,
    .remove   = mcs9835_remove,
    /* resume, suspend are optional */
};

module_init(mcs9835_initialize);
module_exit(mcs9835_finalize);

/****************************************************************************
 *
 * Global variables
 *
 ****************************************************************************/

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

/****************************************************************************
 *
 * PCI core functions
 *
 ****************************************************************************/

/****************************************************************************/

/* 
 * Initialize the device.
 * Executed by the PCI core.
*/
static int DEVINIT_MARK mcs9835_probe(struct pci_dev *dev, 
				      const struct pci_device_id *id)
{
  int rc;
  int i;
  unsigned long joe;

  void __iomem *joe_p;

  u8 joe_data;

  LOG(MCS_INF, "initializing PCI device 0x%x:0x%x\n", 
      dev->vendor, dev->device);

  /* Enable this device */
  rc = pci_enable_device(dev);
  if (rc) {
    LOG(MCS_ERR, "pci_enable_device failed\n");
    return rc;
  }

  for (i=0 ; i < 6; i++) {
    joe = pci_resource_flags(dev, i);
    LOG(MCS_INF, "bar(%d) - flags = 0x%lx - IO = %s\n", 
	i, joe, (joe & IORESOURCE_IO ? "yes " : "no"));
    joe = pci_resource_start(dev, i);
    LOG(MCS_INF, "bar(%d) - start = 0x%lx\n", 
	i, joe);
    joe = pci_resource_end(dev, i);
    LOG(MCS_INF, "bar(%d) - end   = 0x%lx\n", 
	i, joe);
  }

  /* Request all BARs */
  rc = pci_request_regions(dev, DRV_NAME);
  if (rc) {
    LOG(MCS_ERR, "pci_request_regions failed\n");
    return rc;
  }

  // Play with BAR2
  joe_p = pci_iomap(dev, 2, 0);
  if (joe_p == NULL) {
    LOG(MCS_ERR, "pci_iomap(2) failed\n");
    return rc;
  }  
  for (i=0 ; i < 8; i++) {
    joe_data = ioread8(joe_p + i);
    LOG(MCS_INF, "bar(2) + %d = 0x%x\n", 
	i, joe_data);
  }
  pci_iounmap(dev, joe_p);

  // Play with BAR3
  joe_p = pci_iomap(dev, 3, 0);
  if (joe_p == NULL) {
    LOG(MCS_ERR, "pci_iomap(3) failed\n");
    return rc;
  }  
  for (i=0 ; i < 3; i++) {
    joe_data = ioread8(joe_p + i);
    LOG(MCS_INF, "bar(3) + %d = 0x%x\n", 
	i, joe_data);
  }
  pci_iounmap(dev, joe_p);

  return 0;
}

/****************************************************************************/

/* 
 * Finalize the device.
 * Executed by the PCI core.
*/
static void DEVEXIT_MARK mcs9835_remove(struct pci_dev *dev)
{
  LOG(MCS_INF, "finalizing PCI device 0x%x:0x%x\n", 
      dev->vendor, dev->device);

  /* Release all BARs */
  pci_release_regions(dev);

  /* Disable this device */
  pci_disable_device(dev);  
}

/****************************************************************************
 *
 * Module load/unload functions
 *
 ****************************************************************************/

/****************************************************************************/

/*
 * Initialize this kernel module.
 * Executed when the module is loaded.
 */
static int __init mcs9835_initialize(void)
{
  int i;

  printk(KERN_INFO DRV_NAME " [Loading driver %s-%s]\n",
	 DRV_PRODUCT_NUMBER, DRV_RSTATE);

  /* Update initial loglevel list with current settings */
  mcs_set_log_level(mcs_log_level, MCS_ENABLE);
  if (loglevel) {
    mcs_set_log_level(loglevel, MCS_ENABLE);
  }

  /* Display current loglevel configuration in syslog */
  mcs_display_log_levels();

  /* Test of loglevels */
  i = 100;
  LOG(MCS_WRN, "WRN test = 0x%08x\n", i);
  LOG(MCS_INI, "INI test\n");
  i = 0x1234cafe;
  LOG(MCS_IRQ, "IRQ test = 0x%08x\n", i);
  LOG(MCS_DBG, "DBG test\n");

  /* Register driver with PCI core */
  return pci_register_driver(&mcs9835_pci_driver);
}

/****************************************************************************/

/*
 * Finalize this kernel module.
 * Executed when the module is unloaded.
 */
static void __exit mcs9835_finalize(void)
{
  LOG(MCS_INF, "unloading driver\n");

  /* Unregister driver from PCI core */
  pci_unregister_driver(&mcs9835_pci_driver);
}

/****************************************************************************
 *
 * TBD functions
 *
 ****************************************************************************/

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

//module_init(bach_init);
//module_exit(bach_exit);
