#!/bin/sh

die() {
	echo $1
	exit 1
}

FRITZBOX='192.168.178.1'
USER='adam2'
PASSWD='adam2'
FILE='uboot-fritz4040.bin'

ping -q -4 -w 1 -c 1 "$FRITZBOX" &> /dev/null  || die "Fritzbox at $FRITZBOX is not reachable. aborting"

cat << EOS

This will take ages (2 minutes)! If you want a
status bar: Attach a serial to the device and look there.

Note: If this fails, you need to use AVM\' recover
utility to get your box back. However this step
should not damage your box....

Also it doesn't terminate.. :/

(Note: Make sure that you connected the router on
the yellow LAN ports and not the blue WAN).

EOS

ftp -n -4 "$FRITZBOX" << END_SCRIPT
quote USER $USER
quote PASS $PASSWD
quote MEDIA FLSH
binary
passive
put $FILE mtd1
quote check mtd1
bye
END_SCRIPT
exit 0
