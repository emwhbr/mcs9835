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
    echo "Usage: $0 <release|debug|clean> <kdir>"
    echo ""
    echo "release   Build release, no debug support"
    echo "debug     Build with debug support"
    echo "clean     Build clean"
    echo ""
    echo "kdir      Path to kernel sources (for release and debug)"
    exit 1  
}

################################################################
function get_parallel_args()
################################################################
{
    # Check number of CPU's in this machine
    nr_cpus=`cat /proc/cpuinfo | grep processor | wc -l`
    
    # Add one to get number of parallel jobs
    ((nr_jobs=nr_cpus + 1))
    
    echo "-j${nr_jobs}"
    return 0
}

#######################################################################
##############                MAIN                         ############
#######################################################################

### Number of parallel jobs on this machine
PARALLEL_ARGS=`get_parallel_args`

# We need kernel sources for some builds
case "$1" in
    release | debug)
	if [ ! -d "$2" ]; then
	   print_usage_and_die 
	fi
	;;
esac

# Do the build
case "$1" in
    release)
        echo "==[MAKE RELEASE]==="
        make JOBS=${PARALLEL_ARGS} BUILD_TYPE=RELEASE KDIR=$2 all
        ;;

    debug)
        echo "==[MAKE DEBUG]==="
        make JOBS=${PARALLEL_ARGS} BUILD_TYPE=DEBUG KDIR=$2 all
        ;;

    clean)
        echo "==[MAKE CLEAN]==="
        make clean
        ;;

    *)
	print_usage_and_die
        ;;
esac
