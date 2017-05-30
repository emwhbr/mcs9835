# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2017 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

######################################
# Makefile for MCS9835 kernel module #
######################################

DRIVER_NAME = mcs9835

SRC_DRIVER = $(DRIVER_NAME)_core.o \
             $(DRIVER_NAME)_log.o  \
             $(DRIVER_NAME)_cdev.o

# ----- Kernel module build definitions

obj-m     := $(DRIVER_NAME).o
mcs9835-y := $(SRC_DRIVER)

PWD := $(shell pwd)

# ----- Targets

.PHONY : drv_clean

drv: $(DRIVER_NAME)

$(DRIVER_NAME):
ifndef KDIR
	@echo "*** ERROR : Missing argument KDIR (path to kernel sources)"
	exit 1
endif
	$(MAKE) -Wall -Wextra -C $(KDIR) M=$(PWD) modules

drv_clean:
	rm -rf *~
	rm -rf *.o *.ko
	rm -rf .*.cmd
	rm -rf *.mod.c
	rm -rf .tmp_versions
	rm -rf modules.order Module.symvers
