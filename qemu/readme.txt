Create disk image
-----------------
32bit
qemu-img create -f raw /proj/mcs9835/qemu/centos64-i386.img 10G

Install CentOS 6.4 (32bit) on VM
--------------------------------
qemu-system-i386 -enable-kvm \
                 -cpu host \
                 -smp 1 \
                 -m 1024 \
                 -boot d \
                 -hda /proj/mcs9835/qemu/centos64-i386.img \
                 -cdrom /home/emwhbr/Downloads/CentOS-6.4-i386-bin-DVD1.iso

At the end, it will try to reboot the machine, but from the CD once again.
This time in CentOS installer, select boot from local disk.

Finish the installation.
user   	  password
root      mercury
mercury   mercury

QEMU networking 
----------------
Install the following packages (host)
	- bridge-utils	 (for brctl)
	- openvpn	 (for openvpn)
	- uml-utilities  (for tunctl)

Create network scripts (host)
       - qemu-ifup.sh
       - qemu-ifdown.sh

Start the networking (as root) using script qemu-ifup.sh.
Boot the VM (as an ordinary user). When the machine boots up,
assign an IP address to the eth0 interface inside the VM:

$ ifconfig eth0 192.168.100.6 netmask 255.255.255.0

After finished and VM is shutdown, terminate the networking
(as root) using script qemu-ifdown.sh.

Utility script qemu-boot.sh can be used (as normal user) to boot VM.
