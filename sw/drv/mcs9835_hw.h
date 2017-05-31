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

#include <linux/pci_ids.h>

#ifndef __MCS9835_HW_H__
#define __MCS9835_HW_H__

/*
 * PCI
 */
#define MCS9835_VENDOR_ID  PCI_VENDOR_ID_NETMOS
#define MCS9835_DEVICE_ID  PCI_DEVICE_ID_NETMOS_9835

/*
 * Parallel port registers (BAR2 offset)
 */
#define MCS9835_PARPORT_REG_DPR  0x00
#define MCS9835_PARPORT_REG_DSR  0x01
#define MCS9835_PARPORT_REG_DCR  0x02

#endif /* __MCS9835_HW_H__ */
