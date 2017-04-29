Files:
------
config-2.6.32-358.el6.i686
config-2.6.32-358.el6.i686.modified

Download kernel sources:
------------------------
http://vault.centos.org/6.4/os/Source/SPackages/
Package: kernel-2.6.32-358.el6.src.rpm
Extract from RPM: linux-2.6.32-358.el6.tar.bz2

Get kernel configuration:
-------------------------
Extract from VM, /boot/config-2.6.32-358.el6.i686
File: config-2.6.32-358.el6.i686

------------------------------------------------
Disable signature checking for modules on module load.
https://bugzilla.redhat.com/show_bug.cgi?id=613568
File: config-2.6.32-358.el6.i686.modified

Compile kernel (for module build):
----------------------------------
cd linux-2.6.32-358.el6
make clean
make mrproper
cp ../config-2.6.32-358.el6.i686.modified .config
make oldconfig
make vmlinux
