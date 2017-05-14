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

#ifndef __MCS9835_LOG_H__
#define __MCS9835_LOG_H__

#include <linux/kernel.h>

#include "mcs9835_product_info.h"

/****************************************************************************
 *
 * Available loglevels 
 *
 ****************************************************************************/

#define MCS_ALR (0x00000001)  /* Alerts                     */
#define MCS_ERR (0x00000002)  /* Errors                     */
#define MCS_WRN (0x00000004)  /* Warnings                   */
#define MCS_INF (0x00000008)  /* General info               */

#define MCS_INI (0x00000010)  /* Module init & device probe */
#define MCS_REG (0x00000020)  /* Register access            */
#define MCS_SEM (0x00000040)  /* Semaphore/synchronziation  */

#define MCS_CDV (0x00000100)  /* Character device handling  */
#define MCS_IRQ (0x00000200)  /* IRQ handling               */
#define MCS_DMA (0x00000400)  /* DMA handling               */
#define MCS_VMA (0x00000800)  /* Virtual memory mapping     */

#define MCS_DBG (0x00010000)  /* Debug                      */

extern u32 mcs_log_level;

/*
 * Logs information to syslog if the requested loglevel is enabled.
 * If loglevel debug is enabled, add function name and line number
 * to the log string
 */
#define LOG(logid, fmt, args...)		            \
  if (mcs_log_level & logid) do {			    \
      if ((mcs_log_level & MCS_DBG) || (logid == MCS_ERR)) {  \
	printk(KERN_INFO DRV_NAME " (%s):%s():%d: ", #logid, __func__, __LINE__); \
	printk(fmt, ## args);						\
      } else {								\
	printk(KERN_INFO DRV_NAME " (%s): ", #logid);			\
	printk(fmt, ## args);						\
      }									\
  } while(0);

/****************************************************************************
 *
 * Log level status types:
 * MCS_DISABLE - Logging OFF for this level
 * MCS_ENABLE  - Logging ON for this level
 * MCS_TOGGLE  - Change loglevel from current setting (ON->OFF, OFF->ON)
 *
 ****************************************************************************/

typedef enum _MCS_LOGSTATUS_E_ {
  MCS_DISABLE = 0,
  MCS_ENABLE  = 1,
  MCS_TOGGLE  = 2
} MCS_LOGSTATUS_E;

/****************************************************************************
 * 
 * Exported functions
 *
 ****************************************************************************/

extern void mcs_display_log_levels(void);

extern long mcs_set_log_level(const u32 level,
			      const MCS_LOGSTATUS_E status);

#endif /* __MCS9835_LOG_H__ */
