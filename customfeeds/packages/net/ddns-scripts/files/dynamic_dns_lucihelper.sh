#!/bin/sh
# /usr/lib/ddns/dynamic_dns_lucihelper.sh
#
#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
#.2014-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
# This script is used by luci-app-ddns
#
# variables in small chars are read from /etc/config/ddns as parameter given here
# variables in big chars are defined inside these scripts as gloval vars
# variables in big chars beginning with "__" are local defined inside functions only
# set -vx  	#script debugger

. /usr/lib/ddns/dynamic_dns_functions.sh	# global vars are also defined here

usage() {
	cat << EOF

Usage:
 $MYPROG [options] -- command

Commands:
 get_local_ip        using given INTERFACE or NETWORK or SCRIPT or URL
 get_registered_ip   for given FQDN
 verify_dns          given DNS-SERVER
 verify_proxy        given PROXY
 start               start given SECTION
 reload              force running ddns processes to reload changed configuration
 restart             restart all ddns processes

Parameters:
 -6                  => use_ipv6=1          (default 0)
 -d DNS-SERVER       => dns_server=SERVER[:PORT]
 -f                  => force_ipversion=1   (default 0)
 -g                  => is_glue=1           (default 0)
 -i INTERFACE        => ip_interface=INTERFACE; ip_source="interface"
 -l FQDN             => lookup_host=FQDN
 -n NETWORK          => ip_network=NETWORK; ip_source="network"
 -p PROXY            => proxy=[USER:PASS@]PROXY:PORT
 -s SCRIPT           => ip_script=SCRIPT; ip_source="script"
 -t                  => force_dnstcp=1      (default 0)
 -u URL              => ip_url=URL; ip_source="web"
 -S SECTION          SECTION to start

 -h                  => show this help and exit
 -L                  => use_logfile=1    (default 0)
 -v LEVEL            => VERBOSE=LEVEL    (default 0)
 -V                  => show version and exit

EOF
}

usage_err() {
	printf %s\\n "$MYPROG: $@" >&2
	usage >&2
	exit 255
}

# preset some variables, wrong or not set in ddns-functions.sh
SECTION_ID="lucihelper"
LOGFILE="$ddns_logdir/$SECTION_ID.log"
DATFILE="$ddns_rundir/$SECTION_ID.$$.dat"	# save stdout data of WGet and other extern programs called
ERRFILE="$ddns_rundir/$SECTION_ID.$$.err"	# save stderr output of WGet and other extern programs called
DDNSPRG="/usr/lib/ddns/dynamic_dns_updater.sh"
VERBOSE=0		# no console logging
# global variables normally set by reading DDNS UCI configuration
use_syslog=0		# no syslog
use_logfile=0		# no logfile

use_ipv6=0		# Use IPv6 - default IPv4
force_ipversion=0	# Force IP Version - default 0 - No
force_dnstcp=0		# Force TCP on DNS - default 0 - No
is_glue=0		# Is glue record - default 0 - No
use_https=0		# not needed but must be set

while getopts ":6d:fghi:l:n:p:s:S:tu:Lv:V" OPT; do
	case "$OPT" in
		6)	use_ipv6=1;;
		d)	dns_server="$OPTARG";;
		f)	force_ipversion=1;;
		g)	is_glue=1;;
		i)	ip_interface="$OPTARG"; ip_source="interface";;
		l)	lookup_host="$OPTARG";;
		n)	ip_network="$OPTARG"; ip_source="network";;
		p)	proxy="$OPTARG";;
		s)	ip_script="$OPTARG"; ip_source="script";;
		t)	force_dnstcp=1;;
		u)	ip_url="$OPTARG"; ip_source="web";;
		h)	usage; exit 255;;
		L)	use_logfile=1;;
		v)	VERBOSE=$OPTARG;;
		S)	SECTION=$OPTARG;;
		V)	printf %s\\n "ddns-scripts $VERSION"; exit 255;;
		:)	usage_err "option -$OPTARG missing argument";;
		\?)	usage_err "invalid option -$OPTARG";;
		*)	usage_err "unhandled option -$OPT $OPTARG";;
	esac
done
shift $((OPTIND - 1 ))	# OPTIND is 1 based

[ $# -eq 0 ] && usage_err "missing command"

__RET=0
case "$1" in
	get_registered_ip)
		[ -z "$lookup_host" ] && usage_err "command 'get_registered_ip': 'lookup_host' not set" 
		write_log 7 "-----> get_registered_ip IP"
		[ -z "$SECTION" ] || IPFILE="$ddns_rundir/$SECTION.ip"
		IP=""
		get_registered_ip IP
		__RET=$?
		[ $__RET -ne 0 ] && IP=""
		printf "%s" "$IP"
		;;
	verify_dns)
		[ -z "$dns_server" ] && usage_err "command 'verify_dns': 'dns_server' not set" 
		write_log 7 "-----> verify_dns '$dns_server'"
		verify_dns "$dns_server"
		__RET=$?
		;;
	verify_proxy)
		[ -z "$proxy" ] && usage_err "command 'verify_proxy': 'proxy' not set" 
		write_log 7 "-----> verify_proxy '$proxy'"
		verify_proxy "$proxy"
		__RET=$?
		;;
	get_local_ip)
		[ -z "$ip_source" ] && usage_err "command 'get_local_ip': 'ip_source' not set" 
		[ -n "$proxy" -a "$ip_source" = "web" ] && {
			# proxy defined, used for ip_source=web
			export HTTP_PROXY="http://$proxy"
			export HTTPS_PROXY="http://$proxy"
			export http_proxy="http://$proxy"
			export https_proxy="http://$proxy"
		}
		# don't need IP only the return code
		IP=""
		if [ "$ip_source" = "web" -o  "$ip_source" = "script" ]; then
			# we wait only 3 seconds for an
			# answer from "web" or "script"
			write_log 7 "-----> timeout 3 -- get_local_ip IP"
			timeout 3 -- get_local_ip IP
		else
			write_log 7 "-----> get_local_ip IP"
			get_local_ip IP
		fi
		__RET=$?
		;;
	start)
		[ -z "$SECTION" ] &&  usage_err "command 'start': 'SECTION' not set"
		if [ $VERBOSE -eq 0 ]; then	# start in background
			$DDNSPRG -v 0 -S $SECTION -- start &
		else
			$DDNSPRG -v $VERBOSE -S $SECTION -- start
		fi
		;;
	reload)
		$DDNSPRG -- reload
		;;
	restart)
		$DDNSPRG -- stop
		sleep 1
		$DDNSPRG -- start
		;;
	*)
		__RET=255
		;;
esac

# remove out and err file
[ -f $DATFILE ] && rm -f $DATFILE
[ -f $ERRFILE ] && rm -f $ERRFILE
return $__RET
