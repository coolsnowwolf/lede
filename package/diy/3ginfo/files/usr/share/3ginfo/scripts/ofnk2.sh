#!/bin/sh
DEVICE=$(uci -q get 3ginfo.@3ginfo[0].device)
O=$(gcom -d $DEVICE -s /usr/share/3ginfo/scripts/ofnk2.gcom | cut -f2 -d\")
if [ ! -z "$O" ]; then
    A=$(echo $O | awk '{for(i=1;;i=i+4)if(substr($1,i,4)!=""){printf "%c",(index("0123456789ABCDEF",substr($1,i+2,1))-1)*16+(index("0123456789ABCDEF",substr($1,i+3,1))-1)}else exit;}')
    echo $A | sed -e 's/1 Opis.*9 Powrot.*0 Wyjscie//g'
else
    echo "Problem operatora - brak możliwości sprawdzenia"
fi
