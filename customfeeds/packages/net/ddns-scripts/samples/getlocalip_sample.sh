#!/bin/sh
#
# sample script for detecting local IP
# 2014-2015 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
#
# activated inside /etc/config/ddns by setting
#
# option ip_source	'script'
# option ip_script 	'/usr/lib/ddns/getlocalip_sample.sh -6' !!! parameters ALLOWED
#
# the script is executed (not parsed) inside get_local_ip() function
# of /usr/lib/ddns/dynamic_dns_functions.sh
#
# useful when this box is the only DDNS client in the network
# IP adresses of "internal" boxes could be detected with this script
# so no need to install ddns client on every "internal" box
# On IPv6 every internal box normally has it's own external IP
#
# This script should
# 	- return the IP address via stdout	echo -n "...."	!!! without line feed
#	- report errors via stderr		echo "...." >&2
#	- return an error code ('0' for success)	exit 123

case $1 in
	-4)	echo -n "8.8.8.8"		# never append linefeed or simular
		exit 0
		;;				# IP's of Googles public DNS
	-6)	echo -n "2001:4860:4860::8888"
		exit 0
		;;
	*)	echo "$0 - Invalid or missing parameter" >&2
		exit 1
esac
echo "Should never come here" >&2
exit 2
