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

#################################################
# Makefile for MCS9835 library test application #
#################################################

include ../common_defs.mk

TEST_OBJS = $(OBJ_DIR)/de_main.o \
            $(OBJ_DIR)/de_ui.o

COMP_FLAGS_C_TEST_APP   = $(COMP_FLAGS_C)
COMP_FLAGS_CPP_TEST_APP = $(COMP_FLAGS_CPP)

# ----- Test application

TEST_APP_BASENAME = $(OBJ_DIR)/test_lib${LIB_NAME}
TEST_APP_NAME = $(TEST_APP_BASENAME)_$(KIND).$(ARCH_TYPE)

# ----- Linker paths

LD_LIB = -L$(OBJ_DIR)

LIB_DIRS = $(LD_LIB)

# ----- Linker libraries

LIB = -l$(LIB_NAME)
LIBSX = -lstdc++ -lgcc -lpthread -lrt

LIBS =  $(LIB) $(LIBSX)

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h .d

$(OBJ_DIR)/%.o : %.c
	$(CC) $(COMP_FLAGS_C_TEST_APP) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/%.d : %.c
	@$(CC) -MM -MT '$(patsubst %d,%o,$@)' $(COMP_FLAGS_C_TEST_APP) $(INCLUDES) $< > $@

$(OBJ_DIR)/%.o : %.cpp
	$(CPP) $(COMP_FLAGS_CPP_TEST_APP) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/%.d : %.cpp
	@$(CPP) -MM -MT '$(patsubst %d,%o,$@)' $(COMP_FLAGS_CPP_TEST_APP) $(INCLUDES) $< > $@

# ----- Targets

.PHONY : test_clean

-include $(TEST_OBJS:.o=.d)

test : $(TEST_OBJS) $(LIB_FILE_NAME)
	$(CC) -o $(TEST_APP_NAME) $(TEST_OBJS) $(LIB_DIRS) $(LIBS)

test_clean :
	rm -f $(TEST_OBJS) $(TEST_OBJS:.o=.d) $(TEST_APP_BASENAME)* *~
