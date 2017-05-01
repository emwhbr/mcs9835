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

###########################################
# Top level Makefile for MCS9835 software #
###########################################

include common_defs.mk

# ------- Targets

.PHONY : drv lib test all clean

drv:
	@echo " -- [BUILD drv] --"
	@cd ./drv ; make drv

lib:
	@echo " -- [BUILD lib] --"
	@cd ./lib ; make $(JOBS) lib

test:
	@echo " -- [BUILD test] --"
	@cd ./test ; make $(JOBS) test

all: drv lib test

clean:
	@echo " -- [CLEAN top] --"
	@rm -f *~
	@echo " -- [CLEAN drv] --"
	@cd ./drv ; make drv_clean
	@echo " -- [CLEAN lib] --"
	@cd ./lib ; make lib_clean
	@echo " -- [CLEAN test] --"
	@cd ./test ; make test_clean
