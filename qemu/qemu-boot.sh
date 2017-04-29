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

QEMU_i386="/usr/bin/qemu-system-i386"

DVD_DIR=/home/emwhbr/Downloads
IMG_DIR=/proj/mcs9835/qemu

DVD_CENTOS64_i386=${DVD_DIR}/CentOS-6.4-i386-bin-DVD1.iso
IMG_CENTOS64_i386=${IMG_DIR}/centos64-i386.img

CPUS_VM=1
MBYTES_RAM_VM=1024

TAP_IFACE_VM=tap0
MAC_ADDR_VM="52:54:00:12:34:56"

############################################################
function print_usage_and_die()
{
    echo "Usage: $0 {i386 | i386_nonet | i386_dvd}"
    echo ""
    echo "i386        Boot CentOS 6.4 (32bit)"
    echo "i386_nonet  Boot CentOS 6.4 (32bit), no network"
    echo "i386_dvd    Boot CentOS 6.4 (32bit), no network, from DVD"

    exit 1
}

############################################################
function do_boot_i386()
{
    ### VM:
    ### - Build server, CentOS 6.4 (i386, 32bit)
    ### - Boot local disk
    ### - Network enabled

    ### user      password
    ### root      mercury
    ### mercury   mercury

    ${QEMU_i386} -enable-kvm \
                 -cpu host \
            	 -smp ${CPUS_VM} \
            	 -m ${MBYTES_RAM_VM} \
            	 -boot c \
            	 -hda ${IMG_CENTOS64_i386} \
            	 -cdrom ${DVD_CENTOS64_i386} \
            	 -net nic,macaddr=${MAC_ADDR_VM} \
            	 -net tap,ifname=${TAP_IFACE_VM},script=no

    exit 0
}

############################################################
function do_boot_i386_nonet()
{
    ### VM:
    ### - Build server, CentOS 6.4 (i386, 32bit)
    ### - Boot local disk
    ### - Network disabled

    ### user      password
    ### root      mercury
    ### mercury   mercury

    ${QEMU_i386} -enable-kvm \
                 -cpu host \
            	 -smp ${CPUS_VM} \
            	 -m ${MBYTES_RAM_VM} \
            	 -boot c \
            	 -hda ${IMG_CENTOS64_i386} \
            	 -cdrom ${DVD_CENTOS64_i386}

    exit 0
}

############################################################
function do_boot_i386_dvd()
{
    ### VM:
    ### - Build server, CentOS 6.4 (i386, 32bit)
    ### - Boot DVD
    ### - Network disabled

    ### user      password
    ### root      mercury
    ### mercury   mercury

    ${QEMU_i386} -enable-kvm \
                 -cpu host \
            	 -smp ${CPUS_VM} \
            	 -m ${MBYTES_RAM_VM} \
            	 -boot d \
            	 -hda ${IMG_CENTOS64_i386} \
            	 -cdrom ${DVD_CENTOS64_i386}

    exit 0
}

#######################################################################
##############                MAIN                         ############
#######################################################################

### Basic check of arguments
if [ "$#" -ne 1 ]; then
    print_usage_and_die
fi

### Boot VM
case "${1}" in

    i386)
	do_boot_i386
	;;

    i386_nonet)
	do_boot_i386_nonet
	;;

    i386_dvd)
	do_boot_i386_dvd
	;;

    *)
	print_usage_and_die
	;;

esac
