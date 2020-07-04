#!/bin/bash
#
# tftp flash script for wireless routers
#
# Copyright (C) 2004 by Oleg I. Vdovikin <oleg@cs.msu.su>
# Copyright (C) 2005 by Waldemar Brodkorb <wbx@openwrt.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

if [ -z "$1" ] || [ ! -f "$1" ] || [ -z "$2" ]; then
    echo Usage: "$0" firmware vendor
cat << EOF
IMPORTANT:
Notes for Linksys / Asus WL500gx router:
   be sure you have set boot_wait to yes. Power on your router
   after executing this script.

Notes for Asus WL500g router:
   be sure POWER led is flashing (If this is not the case
   poweroff the device, push the reset button & power on
   it again, then release button)

1) connect your pc to the LAN port
2) be sure your link is up and has an address in the
   192.168.1.0/24 address range (and not the 192.168.1.1)

Notes for Toshiba router:
   boot_wait is enabled by default on these units.

1) connect your pc to any of the four LAN ports
2) be sure your link is up and has an address in the
   192.168.10.1/24 address range (and not the 192.168.10.1)
3) run this script (unit will only accept .trx images)
4) Turn unit power on.

EOF
    exit 0
fi
if [ "$2" = "asus" ]; then
echo Confirming IP address setting...
echo -en "get ASUSSPACELINK\x01\x01\xa8\xc0 /dev/null\nquit\n" | tftp 192.168.1.1
echo Flashing 192.168.1.1 using "$1"...
echo -en "binary\nput $1 ASUSSPACELINK\nquit\n" | tftp 192.168.1.1
echo Please wait until leds stops flashing.
elif [ "$2" = "linksys" ]; then
echo Flashing 192.168.1.1 using "$1"...
echo -en "rexmt 1\ntrace\nbinary\nput $1\nquit\n" | tftp 192.168.1.1
echo Please wait until power led stops flashing. Do not poweroff! Then you can login via telnet 192.168.1.1.
elif [ "$2" = "toshiba" ]; then
echo Flashing 192.168.10.1 using "$1"...
echo -en "rexmt 1\ntrace\nbinary\nput $1\nquit\n" | tftp 192.168.10.1
echo Unit will automatically reboot within 5 minutes.  Do not power off.  Then you can login via telnet 192.168.10.1.
fi
