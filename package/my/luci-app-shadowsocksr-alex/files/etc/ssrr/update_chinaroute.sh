#!/bin/sh
TMP_DIR="/tmp"
SSR_DIR="/etc/ssrr"
FILE_NAME="china_route"
echo ready to update china route!!
logger -t alex ready to update china route!!
wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest' | grep ipv4 | grep CN | awk -F\| '{ printf("%s/%d\n", $4, 32-log($5)/log(2)) }' > $TMP_DIR/$FILE_NAME
[ -f $TMP_DIR/$FILE_NAME ] && {
	count=`grep "" -c $TMP_DIR/$FILE_NAME`
	if [ "$count" -gt "1000" ];then
		echo download completed!
		logger -t alex echo download completed!
		cp $TMP_DIR/$FILE_NAME $SSR_DIR/
		echo update completed!
		logger -t alex update completed!
		rm -f $TMP_DIR/$FILE_NAME
	fi
} || {
	logger -t alex download failed!
	echo download failed!
	exit
}