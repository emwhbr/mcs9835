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

/****************************************************************************
 *
 * Macros
 *
 ****************************************************************************/
#define MCS9835_MAX_DEVICES  1

/****************************************************************************
 *
 * PCI driver device private data
 *
 ****************************************************************************/
struct mcs9835_dev {

  struct pci_dev *pci_dev;

  void __iomem *vmem_bar0;
  void __iomem *vmem_bar1;
  void __iomem *vmem_bar2;
  void __iomem *vmem_bar3;

  int init_done;
};

#endif /* __MCS9835_H__ */
