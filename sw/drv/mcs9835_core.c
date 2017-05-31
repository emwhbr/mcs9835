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

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>

#include "mcs9835.h"
#include "mcs9835_product_info.h"
#include "mcs9835_log.h"
#include "mcs9835_cdev.h"
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

static int mcs9835_open_parport(struct inode *inode, 
				struct file  *file);
static int mcs9835_close_parport(struct inode *inode, 
				 struct file  *file);
static ssize_t mcs9835_read_parport(const struct file *file,
				    const char __user *buf, 
				    size_t count,
				    loff_t *pos);

static ssize_t mcs9835_write_parport(const struct file *file,
				     const char __user *buf, 
				    size_t count,
				    loff_t *pos);

static int __init mcs9835_initialize(void);
static void __exit mcs9835_finalize(void);

static void initialize_dev_data(struct mcs9835_dev *dev);

static void parport_write_reg(struct mcs9835_dev *dev,
			      unsigned bar_offset,
			      u8 value);

static u8 parport_read_reg(struct mcs9835_dev *dev,
			   unsigned bar_offset);

static void dump_dev_registers(struct mcs9835_dev *dev);

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

/****************************************************************************
 *
 * Char driver infrastructure
 *
 ****************************************************************************/
struct file_operations mcs9835_fops_parport = {
  .owner   = THIS_MODULE,
  .open    = mcs9835_open_parport,
  .release = mcs9835_close_parport,
  .read    = (void *)mcs9835_read_parport,
  .write   = (void *)mcs9835_write_parport,
};

/****************************************************************************
 *
 * Global variables
 *
 ****************************************************************************/

/* Device private data, allocated in mcs9835_initialize() */
static struct mcs9835_dev *mcs9835_devices = NULL;

/* Keep track of devices */
static int mcs9835_dev_idx = 0;

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

  struct mcs9835_dev *mcs_dev = NULL;

  LOG(MCS_INF, "initialize PCI device 0x%x:0x%x\n", 
      dev->vendor, dev->device);

  /* Check if maximum supported devices reached */
  if (mcs9835_dev_idx >= MCS9835_MAX_DEVICES) {
    LOG(MCS_ERR, "maximum supported devices reached (%d)\n", MCS9835_MAX_DEVICES);
    rc = -EBUSY;
    goto probe_fail_1;
  }
  mcs_dev = &mcs9835_devices[mcs9835_dev_idx];

  /* Enable this device */
  rc = pci_enable_device(dev);
  if (rc) {
    LOG(MCS_ERR, "pci_enable_device failed\n");
    goto probe_fail_1;
  }

  /* 
   * Check that BARs 0-3 are I/O port type 
   * BAR0 : UART-A
   * BAR1 : UART-B
   * BAR2 : Standard parallel port registers
   * BAR3 : Configuration register A, B and ECR
   */
  LOG(MCS_INI, "check BARs configuration\n");
  for (i=0 ; i < 4; i++) {
    if ( !(pci_resource_flags(dev, i) & IORESOURCE_IO) ) {
      LOG(MCS_ERR, "incorrect BAR(%d) configuration\n", i);
      rc = -ENODEV;
      goto probe_fail_2;
    }
  }

  /* Request all BARs */
  rc = pci_request_regions(dev, DRV_NAME);
  if (rc) {
    LOG(MCS_ERR, "pci_request_regions failed\n");
    goto probe_fail_2;
  }

  /* Create virtual mappings for BARs */
  LOG(MCS_INI, "create BARs virtual mappings\n");
  mcs_dev->vmem_bar0 = pci_iomap(dev, 0, 0);
  if (mcs_dev->vmem_bar0 == NULL) {
    LOG(MCS_ERR, "pci_iomap failed for BAR0\n");
    rc = -ENODEV;
    goto probe_fail_3;
  }
  mcs_dev->vmem_bar1 = pci_iomap(dev, 1, 0);
  if (mcs_dev->vmem_bar1 == NULL) {
    LOG(MCS_ERR, "pci_iomap failed for BAR1\n");
    rc = -ENODEV;
    goto probe_fail_3;
  }
  mcs_dev->vmem_bar2 = pci_iomap(dev, 2, 0);
  if (mcs_dev->vmem_bar2 == NULL) {
    LOG(MCS_ERR, "pci_iomap failed for BAR2\n");
    rc = -ENODEV;
    goto probe_fail_3;
  }
  mcs_dev->vmem_bar3 = pci_iomap(dev, 3, 0);
  if (mcs_dev->vmem_bar3 == NULL) {
    LOG(MCS_ERR, "pci_iomap failed for BAR3\n");
    rc = -ENODEV;
    goto probe_fail_3;
  }

  /* Initialize cdev class */
  rc = mcs9835_cdev_initialize(mcs_dev, mcs9835_dev_idx);
  if (rc) {    
    goto probe_fail_3;
  }

  /* Add character device UART-A */
  rc = mcs9835_cdev_create(mcs_dev,
			   mcs9835_dev_idx,
			   MCS9835_CDEV_IDX_UART_A,
			   NULL);
  if (rc) {
    LOG(MCS_ERR, "add character device UART-A failed\n");
    goto probe_fail_4;
  }

  /* Add character device UART-B */
  rc = mcs9835_cdev_create(mcs_dev,
			   mcs9835_dev_idx,
			   MCS9835_CDEV_IDX_UART_B,
			   NULL);
  if (rc) {
    LOG(MCS_ERR, "add character device UART-B failed\n");
    goto probe_fail_5;
  }

  /* Add character device PARPORT */
  rc = mcs9835_cdev_create(mcs_dev,
			   mcs9835_dev_idx,
			   MCS9835_CDEV_IDX_PARPORT,
			   &mcs9835_fops_parport);
  if (rc) {
    LOG(MCS_ERR, "add character device PARPORT failed\n");
    goto probe_fail_6;
  }

  /* Set private driver data pointer*/
  pci_set_drvdata(dev, (void *)mcs_dev);
  mcs_dev->pci_dev = dev;

  /* Another device has been initialized */
  mcs_dev->dev_idx   = mcs9835_dev_idx++;
  mcs_dev->init_done = 1;
  LOG(MCS_INI, "initialize PCI device %d/%d done\n",
      mcs9835_dev_idx, MCS9835_MAX_DEVICES);

  dump_dev_registers(mcs_dev);

  return 0;

probe_fail_6:
  mcs9835_cdev_destroy(mcs_dev, MCS9835_CDEV_IDX_UART_B);

 probe_fail_5:
  mcs9835_cdev_destroy(mcs_dev, MCS9835_CDEV_IDX_UART_A);

 probe_fail_4:
  mcs9835_cdev_finalize(mcs_dev);

 probe_fail_3:
  if (mcs_dev->vmem_bar0 != NULL) {
    pci_iounmap(dev, mcs_dev->vmem_bar0);
  }
  if (mcs_dev->vmem_bar1 != NULL) {
    pci_iounmap(dev, mcs_dev->vmem_bar1);
  }
  if (mcs_dev->vmem_bar2 != NULL) {
    pci_iounmap(dev, mcs_dev->vmem_bar2);
  }
  if (mcs_dev->vmem_bar3 != NULL) {
    pci_iounmap(dev, mcs_dev->vmem_bar3);
  }
  pci_release_regions(dev); /* Release all BARs */

 probe_fail_2:
  pci_disable_device(dev); /* Disable this device */

 probe_fail_1:
  return rc;
}

/****************************************************************************/

/* 
 * Finalize the device.
 * Executed by the PCI core.
*/
static void DEVEXIT_MARK mcs9835_remove(struct pci_dev *dev)
{
  struct mcs9835_dev *mcs_dev = NULL;

  LOG(MCS_INF, "finalize PCI device 0x%x:0x%x\n", 
      dev->vendor, dev->device);

  /* Get private driver data pointer */
  mcs_dev = pci_get_drvdata(dev);
  if ( (mcs_dev != NULL) &&
       (mcs_dev-> init_done) ) {

    /* Remove character devices */
    mcs9835_cdev_destroy(mcs_dev, MCS9835_CDEV_IDX_UART_A);
    mcs9835_cdev_destroy(mcs_dev, MCS9835_CDEV_IDX_UART_B);
    mcs9835_cdev_destroy(mcs_dev, MCS9835_CDEV_IDX_PARPORT);
    mcs9835_cdev_finalize(mcs_dev);

    /* Unmap all BARs */
    pci_iounmap(dev, mcs_dev->vmem_bar0);
    pci_iounmap(dev, mcs_dev->vmem_bar1);
    pci_iounmap(dev, mcs_dev->vmem_bar2);
    pci_iounmap(dev, mcs_dev->vmem_bar3);

    /* Release all BARs */
    pci_release_regions(dev);

    /* Disable this device */
    pci_disable_device(dev);

    LOG(MCS_INI, "finalize PCI device %d/%d done\n",
	mcs9835_dev_idx, MCS9835_MAX_DEVICES);

    mcs_dev-> init_done = 0;
    mcs9835_dev_idx--;
  }
}

/****************************************************************************
 *
 * File operation functions
 *
 ****************************************************************************/

/****************************************************************************/

static int mcs9835_open_parport(struct inode *inode, 
				struct file  *file)
{
  struct mcs9835_dev *mcs_dev = NULL;

  /*
   * Get device private data
   * and check that device is ok.
   */
  mcs_dev = container_of(inode->i_cdev, 
			 struct mcs9835_dev, 
			 chr[MCS9835_CDEV_IDX_PARPORT].cdev);
  if (mcs_dev == NULL) {    
    return -ENODEV;
  }

  if (!mcs_dev->init_done) {
    return -ENODEV;
  }

  /* Store device data for other methods */
  file->private_data = mcs_dev;

  LOG(MCS_CDV, "open /dev/%s_%d_%d\n",
      DRV_NAME, mcs_dev->dev_idx, MCS9835_CDEV_IDX_PARPORT);

  return 0;
}

/****************************************************************************/

static int mcs9835_close_parport(struct inode *inode, 
				 struct file  *file)
{
  struct mcs9835_dev *mcs_dev = NULL;

  /* Get device private data */
  mcs_dev = file->private_data;
  if (mcs_dev == NULL) {    
    return -ENODEV;
  }

  LOG(MCS_CDV, "close /dev/%s_%d_%d\n",
      DRV_NAME, mcs_dev->dev_idx, MCS9835_CDEV_IDX_PARPORT);

  return 0;
}

/****************************************************************************/

static ssize_t mcs9835_read_parport(const struct file *file,
				    const char __user *buf, 
				    size_t count,
				    loff_t *pos)
{
  struct mcs9835_dev *mcs_dev = NULL;
  u8 data;

  /* Get device private data */
  mcs_dev = file->private_data;
  if (mcs_dev == NULL) {    
    return -ENODEV;
  }

  /* Check user input */
  if (count != 1) {
    return -EINVAL;
  }

  /* Read data from port */
  data = parport_read_reg(mcs_dev, MCS9835_PARPORT_REG_DSR);

  /* Return data to user */
  if (copy_to_user((u8 *)buf, &data, 1)) {
    return -EFAULT;
  }

  return 1;
}

/****************************************************************************/

static ssize_t mcs9835_write_parport(const struct file *file,
				     const char __user *buf, 
				     size_t count,
				     loff_t *pos)
{
  struct mcs9835_dev *mcs_dev = NULL;
  u8 data;

  /* Get device private data */
  mcs_dev = file->private_data;
  if (mcs_dev == NULL) {    
    return -ENODEV;
  }

  /* Check user input */
  if (count != 1) {
    return -EINVAL;
  }
  
  /* Get data from user */
  if (copy_from_user(&data, (u8 *)buf, 1)) {
    return -EFAULT;
  }

  /* Write data to port */
  parport_write_reg(mcs_dev,
		    MCS9835_PARPORT_REG_DPR,
		    data);
  return 1;
}

/****************************************************************************
 *
 * Module load/unload functions
 *
 ****************************************************************************/
module_init(mcs9835_initialize);
module_exit(mcs9835_finalize);

/****************************************************************************/

/*
 * Initialize this kernel module.
 * Executed when the module is loaded.
 */
static int __init mcs9835_initialize(void)
{
  int i;
  int rc;

  printk(KERN_INFO DRV_NAME " loading driver %s-%s\n",
	 DRV_PRODUCT_NUMBER, DRV_RSTATE);

  /* Update initial loglevel list with current settings */
  mcs_set_log_level(mcs_log_level, MCS_ENABLE);
  if (loglevel) {
    mcs_set_log_level(loglevel, MCS_ENABLE);
  }

  /* Display current loglevel configuration in syslog */
  mcs_display_log_levels();

  /* Allocate device data */
  mcs9835_devices = kmalloc(MCS9835_MAX_DEVICES * sizeof(struct mcs9835_dev), GFP_KERNEL);
  if (mcs9835_devices == NULL) {
    LOG(MCS_ERR, "allocate device data failed\n");
    rc = -ENOMEM;
    goto init_fail;
  }
  memset(mcs9835_devices, 0, MCS9835_MAX_DEVICES * sizeof(struct mcs9835_dev));

  /* Initialize device data */
  for (i=0; i < MCS9835_MAX_DEVICES; i++) {
    initialize_dev_data(&mcs9835_devices[i]);
  }

  /* Register driver with PCI core */
  return pci_register_driver(&mcs9835_pci_driver);

 init_fail:
  return rc;
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

  /* Free all device data */
  if (mcs9835_devices != NULL) {
    kfree(mcs9835_devices);
  }
}

/****************************************************************************
 *
 * Support functions
 *
 ****************************************************************************/

/****************************************************************************/

static void initialize_dev_data(struct mcs9835_dev *dev)
{
  int i;

  /* PCI */
  dev->pci_dev = NULL;

  dev->vmem_bar0 = NULL;
  dev->vmem_bar1 = NULL;
  dev->vmem_bar2 = NULL;
  dev->vmem_bar3 = NULL;

  /* Character devices */
  dev->class = NULL;
  for (i=0; i < MCS9835_MAX_CDEVS; i++) {
    memset(&dev->chr[i], 0, sizeof(struct mcs9835_char));   
  }

  /* Misc */
  dev->dev_idx   = 0;
  dev->init_done = 0;
}

/****************************************************************************/

static void parport_write_reg(struct mcs9835_dev *dev,
			      unsigned bar_offset,
			      u8 value)
{
  LOG(MCS_REG, "PARPORT[%u] <- 0x%02x\n", bar_offset, value);

  /* Parallel port is at BAR2 */
  iowrite8(value, dev->vmem_bar2 + bar_offset);
}

/****************************************************************************/

static u8 parport_read_reg(struct mcs9835_dev *dev,
			   unsigned bar_offset)
{
  u8 value;

  /* Parallel port is at BAR2 */
  value = ioread8(dev->vmem_bar2 + bar_offset);

  LOG(MCS_REG, "PARPORT[%u] -> 0x%02x\n", bar_offset, value);

  return value;
}


/****************************************************************************/

static void dump_dev_registers(struct mcs9835_dev *dev)
{
  int i;
  u8 reg_data;

  LOG(MCS_INF, "BAR0 - UART-A\n");
  for (i=0; i < 8; i++) {
    reg_data = ioread8(dev->vmem_bar0 + i);
    LOG(MCS_INF, "  [%d] = 0x%02x\n", i, reg_data);
  }

  LOG(MCS_INF, "BAR1 - UART-B\n");
  for (i=0; i < 8; i++) {
    reg_data = ioread8(dev->vmem_bar1 + i);
    LOG(MCS_INF, "  [%d] = 0x%02x\n", i, reg_data);
  }

  LOG(MCS_INF, "BAR2 - SPPR\n");
  for (i=0; i < 8; i++) {
    reg_data = ioread8(dev->vmem_bar2 + i);
    LOG(MCS_INF, "  [%d] = 0x%02x\n", i, reg_data);
  }

  LOG(MCS_INF, "BAR3\n");
  for (i=0; i < 3; i++) {
    reg_data = ioread8(dev->vmem_bar3 + i);
    LOG(MCS_INF, "  [%d] = 0x%02x\n", i, reg_data);
  }
}
