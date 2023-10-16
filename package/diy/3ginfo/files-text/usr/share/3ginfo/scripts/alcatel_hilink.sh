#!/bin/sh

IP=$1
[ -z "$IP" ] && exit 0
[ -e /usr/bin/wget ] || exit 0

getvaluen() {
	echo $(sed 's!.*"'$2'":\([^,]*\).*!\1!g' /tmp/$1)
}

getvalue() {
	echo $(sed 's!.*"'$2'":"\([^"]*\).*!\1!g' /tmp/$1)
}

rand=`awk 'BEGIN{srand();print int(rand()*(99000-1000))+1000 }'`

files="getWanInfo getImgInfo getSysteminfo"
for f in $files; do
	wget -t 3 -O /tmp/$f "http://$IP/goform/$f?rand=$rand" >/dev/null 2>&1
done

MODEN=$(getvaluen getWanInfo "network_type")
case $MODEN in
	1)  MODE="GPRS";;
	2)  MODE="EDGE";;
	3)  MODE="HSDPA";;
	4)  MODE="HSUPA";;
	5)  MODE="UMTS";;
	9)  MODE="GSM";;
	11)  MODE="LTE";;
	12)  MODE="HSPA+";;
	13)  MODE="DC HSPA+";;
	*)  MODE="-";;
esac
echo "^SYSINFOEX:x,x,x,x,,x,\"$MODE\",$MODEN,\"$MODE\""

OPER=$(getvalue getWanInfo "network_name")
echo "+COPS: 0,0,\"$OPER\",x"

SIGNAL=$(getvaluen getImgInfo "signal")
case $SIGNAL in
	1)  CSQ="6";;
	2)  CSQ="12";;
	3)  CSQ="18";;
	4)  CSQ="24";;
	5)  CSQ="31";;
	*)  CSQ="0";;
esac
echo "+CSQ: $CSQ,99"

MODEL=$(getvalue getSysteminfo "dev_name")
echo "DEVICE:Alcatel $MODEL"

if [ "x$2" != "xdebug" ]; then
	for f in $files; do
		rm /tmp/$f
	done
fi

exit 0
