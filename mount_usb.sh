#! /bin/sh

sleep 1
while [ ! -e /dev/sda1 ] && [ ! -e /dev/sdb1 ]; do
	sleep 1
	echo "retry...find /dev/sda1 or sdb1"
done

if [ -e /tmp/mmc ]; then
	##----------------(if used MMC from pinetd.c)##
	umount -f /tmp/mmc
fi

echo "make /tmp/usb"
mkdir -p /tmp/usb
if [ -e /dev/sda1 ]; then
	echo "mount /dev/sda1 on /tmp/usb"
	mount -t vfat /dev/sda1 /tmp/usb
elif [ -e /dev/sdb1 ]; then
	echo "mount /dev/sdb1 on /tmp/usb"
	mount -t vfat /dev/sdb1 /tmp/usb
fi
