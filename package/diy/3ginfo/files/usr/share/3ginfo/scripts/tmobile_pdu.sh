#!/bin/sh
. /usr/share/3ginfo/scripts/functions.sh

DEVICE=$(uci -q get 3ginfo.@3ginfo[0].device)
O=$(gcom -d $DEVICE -s /usr/share/3ginfo/scripts/tmobile.gcom | cut -f2 -d\")
if [ ! -z "$O" ]; then
	pdu2txt "$O" | tr '\n' ' ' | sed -e 's/1 Opis 9 Powrot 0 Wyjscie//g'
else
	echo "Problem operatora - brak możliwości sprawdzenia"
fi
