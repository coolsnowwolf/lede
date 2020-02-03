#!/bin/sh

[ -f /etc/opkg.conf ] && grep -q "src\/" /etc/opkg.conf || exit 0

echo -e "# Old feeds from previous image\n# Uncomment to reenable\n" >> /etc/opkg/customfeeds.conf
sed -n "s/.*\(src\/.*\)/# \1/p" /etc/opkg.conf >> /etc/opkg/customfeeds.conf
sed -i "/.*src\/.*/d" /etc/opkg.conf

exit 0
