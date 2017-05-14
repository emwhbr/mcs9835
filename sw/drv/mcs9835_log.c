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

#include <linux/errno.h>

#include "mcs9835_log.h"


/****************************************************************************
 *
 * Types and definitions
 *
 ****************************************************************************/

/* Length definitions */
#define MCS_MAX_LOGLEVELS  12
#define MCS_MAX_LOGNAME    32

/* Helper macro to initially define the logging list entries */
#define ADDLEVEL(x) {x, MCS_DISABLE, #x}

/* Loglevel info struct */
typedef struct _MCS_LOGINFO_T_ {
  u32             level;
  MCS_LOGSTATUS_E status;
  char            name[MCS_MAX_LOGNAME];
} MCS_LOGINFO_T;

/****************************************************************************
 *
 * Global variables
 *
 ****************************************************************************/

MCS_LOGINFO_T log_list[] = {
  ADDLEVEL(MCS_ALR),
  ADDLEVEL(MCS_ERR),
  ADDLEVEL(MCS_WRN),
  ADDLEVEL(MCS_INF),
  
  ADDLEVEL(MCS_INI),
  ADDLEVEL(MCS_REG),
  ADDLEVEL(MCS_SEM),

  ADDLEVEL(MCS_CDV),
  ADDLEVEL(MCS_IRQ),
  ADDLEVEL(MCS_DMA),
  ADDLEVEL(MCS_VMA),
    
  ADDLEVEL(MCS_DBG) 
};

/*
 * Initial active loglevels
 *
 * This setting should normally be 
 * 0x0000000F = (MCS_ALR | MCS_ERR | MCS_WRN | MCS_INF)

 * Active loglevels may be overridden by:
 * Module build - This definition
 * Module load  - Module parameter loglevel
 */
u32 mcs_log_level = MCS_ALR | 
                    MCS_ERR | 
                    MCS_WRN | 
                    MCS_INF;

/****************************************************************************
 *
 * Exported functions
 *
 ****************************************************************************/

/****************************************************************************/

void mcs_display_log_levels(void)
{
  int i = 0;

  printk(KERN_INFO DRV_NAME "  Log mask: [0x%08x]\n",
	 mcs_log_level);

  for (i = 0; i < MCS_MAX_LOGLEVELS; i++) {
    if (log_list[i].status == MCS_ENABLE) {
      printk(KERN_INFO DRV_NAME "  0x%05x\tON\t%s\n", 
	     log_list[i].level, log_list[i].name);
    } else {
      printk(KERN_INFO DRV_NAME "  0x%05x\t\t%s\n", 
	     log_list[i].level, log_list[i].name);    
    }
  }
}

/****************************************************************************/

long mcs_set_log_level(const u32 level,
		       const MCS_LOGSTATUS_E status)
{
  int i = 0;

  /* Parameter check */
  if ( (status < MCS_DISABLE) ||
       (status > MCS_TOGGLE) ) {
    return -EINVAL;
  }
  
  /* Find requested loglevel to modify */
  for (i = 0; i < MCS_MAX_LOGLEVELS; i++) {  
    if (log_list[i].level == (level & log_list[i].level)) {
      /* Set new status for level */
      if (status == MCS_TOGGLE)  {
	/* Toggle status from previous state */
	log_list[i].status = (++log_list[i].status % 2);
      } else {
	/* Set explicit status */
	log_list[i].status = status;
      }
      /* Toggle current loglevel bit in bitfield */
      if (log_list[i].status == MCS_DISABLE) {
	/* Disable logging by clearing bit */
	mcs_log_level &= ~log_list[i].level;
      } else {
	/* Enable logging by setting bit */
	mcs_log_level |= log_list[i].level;
      }     
    }
  }
  
  return 0;
}
