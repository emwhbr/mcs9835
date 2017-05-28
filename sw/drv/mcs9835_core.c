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
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "mcs9835.h"
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

static void initialize_dev_data(struct mcs9835_dev *dev);
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

module_init(mcs9835_initialize);
module_exit(mcs9835_finalize);

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

  /* Set private driver data pointer*/
  pci_set_drvdata(dev, (void *)mcs_dev);
  mcs_dev->pci_dev = dev;

  /* Another device has been initialized */
  mcs_dev-> init_done = 1;
  mcs9835_dev_idx++;
  LOG(MCS_INI, "initialize PCI device %d/%d done\n",
      mcs9835_dev_idx, MCS9835_MAX_DEVICES);

  dump_dev_registers(mcs_dev);

  return 0;

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
  dev->pci_dev = NULL;

  dev->vmem_bar0 = NULL;
  dev->vmem_bar1 = NULL;
  dev->vmem_bar2 = NULL;
  dev->vmem_bar3 = NULL;

  dev->init_done = 0;
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
