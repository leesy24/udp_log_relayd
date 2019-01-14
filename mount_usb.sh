#! /bin/sh

sleep 1
while [ ! -e /dev/sdb1 ]; do
	sleep 1
done

echo "mount /dev/sdb1 on /tmp/usb."
mkdir -p /tmp/usb
mount -t vfat /dev/sdb1 /tmp/usb
