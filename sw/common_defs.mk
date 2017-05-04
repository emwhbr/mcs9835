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

############################################################
# Common Makefile definitions for MCS9835 library and test #
############################################################

# ----- Toolchain setup
ARCH_TYPE = i386
CFLAGS_ARCH_TUNING=-m32
TC_PREFIX=

AR  = $(TC_PREFIX)ar
CC  = $(TC_PREFIX)gcc
CPP = $(TC_PREFIX)g++
AS  = $(TC_PREFIX)gcc
LD  = $(TC_PREFIX)gcc

# ----- Bonden naive setup

ifeq "$(BUILD_TYPE)" "RELEASE"
	OPTIMIZE = -O3
	KIND = rel
else 
	OPTIMIZE = -O0 -g3
	KIND = dbg
	DEBUG_PRINTS = -DDEBUG_PRINTS
endif

OBJ_DIR = ../obj
INC_DIR = ../lib
TEST_DIR = ../test

# The library is actual handling Serial Parallel I/O.
# Let's call it SPIO to get a short and simple name.
LIB_NAME = spio
LIB_BASENAME = $(OBJ_DIR)/lib${LIB_NAME}
LIB_FILE_NAME = $(LIB_BASENAME).so

# ----- Compiler flags

CFLAGS = -Wall -Wextra -Dlinux -Werror -Dlinux -Wno-packed-bitfield-compat
CFLAGS += $(CFLAGS_ARCH_TUNING)
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEBUG_PRINTS)

COMP_FLAGS = $(CFLAGS) -c
COMP_FLAGS_C = $(COMP_FLAGS) -std=c99
COMP_FLAGS_CPP = $(COMP_FLAGS) -std=c++0x -pedantic -Wno-long-long -Wno-variadic-macros

# ----- Compiler includes

LIB_INCLUDE  = -I$(INC_DIR)

INCLUDES = $(LIB_INCLUDE)
