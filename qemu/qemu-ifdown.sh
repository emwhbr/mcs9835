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
host_nic=eth1
host_ip=192.168.100.17

# Delete the bridge and TAP interface
# Restore NIC
ifconfig ${host_nic} down
ifconfig ${host_nic} -promisc
ifconfig ${host_nic} ${host_ip} up
ifconfig br0 down
brctl delbr br0
tunctl -d tap0

exit 0
