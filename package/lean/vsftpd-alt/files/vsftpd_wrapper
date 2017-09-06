#!/bin/sh
# Copyright (C) 2005-2016 Weijie Gao <hackpascal@gmail.com>

. $IPKG_INSTROOT/lib/functions.sh

VAR=/var/run/vsftpd
CONF=$VAR/vsftpd.conf
listen_addr=
enabled=
param=""
listen4=
listen6=
protocol=$1

config_load vsftpd

if ! [ -f $CONF ]; then
	echo "/usr/bin/vsftpd_prepare must be executed before this script"
	exit 1
fi

case $protocol in
ipv6)
	config_get enabled listen enable6

	if [ x$enabled != x1 ]; then exit 0; fi

	config_get listen_addr listen ipv6
	param="6"
	listen4="-olisten=NO"
	listen6="-olisten_ipv6=YES"

	if [ -z $listen_addr ]; then listen_addr="::"; fi
;;
*)
	config_get enabled listen enable4

	if [ x$enabled != x1 ]; then exit 0; fi

	config_get listen_addr listen ipv4
	listen4="-olisten=YES"
	listen6="-olisten_ipv6=NO"

	if [ -z $listen_addr ]; then listen_addr="0.0.0.0"; fi
esac

exec /usr/sbin/vsftpd "-olisten_address${param}=${listen_addr}" ${listen4} ${listen6} $CONF

exit 1
