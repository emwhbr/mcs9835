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

#ifndef __MCS9835_H__
#define __MCS9835_H__

#include <linux/pci.h>
#include <linux/cdev.h>

/****************************************************************************
 *
 * Macros
 *
 ****************************************************************************/
/* Max supported devices */
#define MCS9835_MAX_DEVICES  1

/* Character devices */
#define MCS9835_MAX_CDEVS  3

#define MCS9835_CDEV_IDX_UART_A   0
#define MCS9835_CDEV_IDX_UART_B   1
#define MCS9835_CDEV_IDX_PARPORT  2

/****************************************************************************
 *
 * Support types
 *
 ****************************************************************************/
struct mcs9835_char {
  struct cdev cdev;
  dev_t       cdevno;
  int         have_cdev;
};

/****************************************************************************
 *
 * PCI driver device private data
 *
 ****************************************************************************/
struct mcs9835_dev {

  /* PCI */
  struct pci_dev *pci_dev;

  void __iomem *vmem_bar0;
  void __iomem *vmem_bar1;
  void __iomem *vmem_bar2;
  void __iomem *vmem_bar3;

  /* Character devices */
  struct class *class;

  struct mcs9835_char chr[MCS9835_MAX_CDEVS];

  /* Misc */
  int dev_idx;
  int init_done;
};

#endif /* __MCS9835_H__ */
