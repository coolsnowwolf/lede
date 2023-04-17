#!/bin/sh

new_version=`curl -sL "https://github.com/frainzy1477/luci-app-clash/tags"| grep "/frainzy1477/luci-app-clash/releases/"| head -n 1| awk -F "/tag/" '{print $2}'| sed 's/\">//'`
if [ "$?" -eq "0" ]; then
rm -rf /usr/share/clash/new_luci_version
if [ $new_version ]; then
echo $new_version > /usr/share/clash/new_luci_version 2>&1 & >/dev/null
elif [ $new_version =="" ]; then
echo 0 > /usr/share/clash/new_luci_version 2>&1 & >/dev/null
fi
fi
 