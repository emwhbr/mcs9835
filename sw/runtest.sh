#!/bin/bash
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

################################################################
function print_usage_and_die()
################################################################
{
    echo "Usage: $0 <rel|dbg>"
    echo ""
    echo "rel   Run test executable, no debug support"
    echo "dbg   Run test executable with debug support"
    exit 1  
}

#######################################################################
##############                MAIN                         ############
#######################################################################

case "$1" in
    rel | dbg)
	export LD_LIBRARY_PATH=./obj/
	./obj/test_libspio_$1.i386
        ;;

     *)
        print_usage_and_die
        ;;
esac
