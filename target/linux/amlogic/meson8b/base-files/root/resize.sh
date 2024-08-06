#!/bin/bash

parted /dev/mmcblk1 resizepart 2 100%
losetup /dev/loop0 /dev/mmcblk1p2
resize2fs -f /dev/loop0
echo "# Put your custom commands here that should be executed once
# the system init finished. By default this file does nothing.

exit 0">/etc/rc.local
rm -rf /root/resize.sh && reboot
