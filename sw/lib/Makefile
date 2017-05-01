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

################################
# Makefile for MCS9835 library #
################################

include ../common_defs.mk

LIB_OBJS = $(OBJ_DIR)/de.o \
           $(OBJ_DIR)/de_core.o \
           $(OBJ_DIR)/de_exception.o \
           $(OBJ_DIR)/de_io.o

COMP_FLAGS_C_LIB   = $(COMP_FLAGS_C) -fPIC
COMP_FLAGS_CPP_LIB = $(COMP_FLAGS_CPP) -fPIC

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h .d

$(OBJ_DIR)/%.o : %.c
	$(CC) $(COMP_FLAGS_C_LIB) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/%.d : %.c
	@$(CC) -MM -MT '$(patsubst %d,%o,$@)' $(COMP_FLAGS_C_LIB) $(INCLUDES) $< > $@

$(OBJ_DIR)/%.o : %.cpp
	$(CPP) $(COMP_FLAGS_CPP_LIB) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/%.d : %.cpp
	@$(CPP) -MM -MT '$(patsubst %d,%o,$@)' $(COMP_FLAGS_CPP) $(INCLUDES) $< > $@

# ------ Targets

.PHONY : lib_clean

-include $(LIB_OBJS:.o=.d)

lib : $(LIB_OBJS)
	$(CPP) -shared -o $(LIB_FILE_NAME) $(LIB_OBJS)

lib_clean :
	rm -f $(LIB_OBJS) $(LIB_OBJS:.o=.d) $(LIB_BASENAME)* *~
