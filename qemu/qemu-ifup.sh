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

# Debug this script, uncomment row below
#set -x

# Check if root
if [[ $EUID -ne 0 ]]; then
   echo "You must be root to do this"
   exit 1
fi

# Config parameters
host_usr=emwhbr
host_nic=eth1
host_ip=192.168.100.17

# Create the bridge and TAP interface
# Add to NIC
ifconfig ${host_nic} down
ifconfig ${host_nic} 0.0.0.0 promisc up
tunctl -u ${host_usr} -t tap0
ifconfig tap0 0.0.0.0 up
brctl addbr br0
brctl addif br0 ${host_nic}
brctl addif br0 tap0
ifconfig br0 ${host_ip} netmask 255.255.255.0

exit 0
