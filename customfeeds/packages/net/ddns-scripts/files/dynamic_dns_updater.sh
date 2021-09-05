#!/bin/sh
# /usr/lib/ddns/dynamic_dns_updater.sh
#
#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
# Original written by Eric Paul Bishop, January 2008
# (Loosely) based on the script on the one posted by exobyte in the forums here:
# http://forum.openwrt.org/viewtopic.php?id=14040
# extended and partial rewritten
#.2014-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
#
# variables in small chars are read from /etc/config/ddns
# variables in big chars are defined inside these scripts as global vars
# variables in big chars beginning with "__" are local defined inside functions only
# set -vx  	#script debugger

. $(dirname $0)/dynamic_dns_functions.sh	# global vars are also defined here

usage() {
	cat << EOF

Usage:
 $MYPROG [options] -- command

Commands:
start                Start SECTION or NETWORK or all
stop                 Stop NETWORK or all

Parameters:
 -n NETWORK          Start/Stop sections in background monitoring NETWORK, force VERBOSE=0
 -S SECTION          SECTION to start
                     use either -N NETWORK or -S SECTION

 -h                  show this help and exit
 -V                  show version and exit
 -v LEVEL            VERBOSE=LEVEL (default 1)
                        '0' NO output to console
                        '1' output to console
                        '2' output to console AND logfile
                            + run once WITHOUT retry on error
                        '3' output to console AND logfile
                            + run once WITHOUT retry on error
                            + NOT sending update to DDNS service

EOF
}

usage_err() {
	printf %s\\n "$MYPROG: $@" >&2
	usage >&2
	exit 1
}

while getopts ":hv:n:S:V" OPT; do
	case "$OPT" in
		h)	usage; exit 0;;
		v)	VERBOSE=$OPTARG;;
		n)	NETWORK=$OPTARG;;
		S)	SECTION_ID=$OPTARG;;
		V)	printf %s\\n "ddns-scripts $VERSION"; exit 0;;
		:)	usage_err "option -$OPTARG missing argument";;
		\?)	usage_err "invalid option -$OPTARG";;
		*)	usage_err "unhandled option -$OPT $OPTARG";;
	esac
done
shift $((OPTIND - 1 ))	# OPTIND is 1 based

[ -n "$NETWORK" -a -n "$SECTION_ID" ] && usage_err "use either option '-N' or '-S' not both"
[ $# -eq 0 ] && usage_err "missing command"
[ $# -gt 1 ] && usage_err "to much commands"

case "$1" in
	start)
		if [ -n "$NETWORK" ]; then
			start_daemon_for_all_ddns_sections "$NETWORK"
			exit 0
		fi
		if [ -z "$SECTION_ID" ]; then
			start_daemon_for_all_ddns_sections
			exit 0
		fi
		;;
	stop)
		if [ -n "$INTERFACE" ]; then
			stop_daemon_for_all_ddns_sections "$NETWORK"
			exit 0
		else
			stop_daemon_for_all_ddns_sections
			exit 0
		fi
		exit 1
		;;
	reload)
		killall -1 dynamic_dns_updater.sh 2>/dev/null
		exit $?
		;;
	*)	usage_err "unknown command - $1";;
esac

# set file names
PIDFILE="$ddns_rundir/$SECTION_ID.pid"	# Process ID file
UPDFILE="$ddns_rundir/$SECTION_ID.update"	# last update successful send (system uptime)
DATFILE="$ddns_rundir/$SECTION_ID.dat"	# save stdout data of WGet and other extern programs called
ERRFILE="$ddns_rundir/$SECTION_ID.err"	# save stderr output of WGet and other extern programs called
IPFILE="$ddns_rundir/$SECTION_ID.ip"	#
LOGFILE="$ddns_logdir/$SECTION_ID.log"	# log file

# VERBOSE > 1 delete logfile if exist to create an empty one
# only with this data of this run for easier diagnostic
# new one created by write_log function
[ $VERBOSE -gt 1 -a -f $LOGFILE ] && rm -f $LOGFILE

# TRAP handler
trap "trap_handler 0 \$?" 0	# handle script exit with exit status
trap "trap_handler 1"  1	# SIGHUP	Hangup / reload config
trap "trap_handler 2"  2	# SIGINT	Terminal interrupt
trap "trap_handler 3"  3	# SIGQUIT	Terminal quit
# trap "trap_handler 9"  9	# SIGKILL	no chance to trap
trap "trap_handler 15" 15	# SIGTERM	Termination

################################################################################
# Leave this comment here, to clearly document variable names that are expected/possible
# Use load_all_config_options to load config options, which is a much more flexible solution.
#
# config_load "ddns"
# config_get <variable> $SECTION_ID <option>
#
# defined options (also used as variable):
#
# enabled	self-explanatory
# interface 	network interface used by hotplug.d i.e. 'wan' or 'wan6'
#
# service_name	Which DDNS service do you use or "custom"
# update_url	URL to use to update your "custom" DDNS service
# update_script SCRIPT to use to update your "custom" DDNS service
#
# lookup_host	FQDN of ONE of your at DDNS service defined host / required to validate if IP update happen/necessary
# domain 	Nomally your DDNS hostname / replace [DOMAIN] in update_url
# username 	Username of your DDNS service account / urlenceded and replace [USERNAME] in update_url
# password 	Password of your DDNS service account / urlencoded and replace [PASSWORD] in update_url
# param_enc	Optional parameter for (later) usage  / urlencoded and replace [PARAMENC] in update_url
# param_opt	Optional parameter for (later) usage  / replace [PARAMOPT] in update_url
#
# use_https	use HTTPS to update DDNS service
# cacert	file or directory where HTTPS can find certificates to verify server; 'IGNORE' ignore check of server certificate
#
# use_syslog	log activity to syslog
#
# ip_source	source to detect current local IP ('network' or 'web' or 'script' or 'interface')
# ip_network	local defined network to read IP from i.e. 'wan' or 'wan6'
# ip_url	URL to read local address from i.e. http://checkip.dyndns.com/ or http://checkipv6.dyndns.com/
# ip_script	full path and name of your script to detect local IP
# ip_interface	physical interface to use for detecting
#
# check_interval	check for changes every  !!! checks below 10 minutes make no sense because the Internet
# check_unit		'days' 'hours' 'minutes' !!! needs about 5-10 minutes to sync an IP-change for an DNS entry
#
# force_interval	force to send an update to your service if no change was detected
# force_unit		'days' 'hours' 'minutes' !!! force_interval="0" runs this script once for use i.e. with cron
#
# retry_interval	if error was detected retry in
# retry_unit		'days' 'hours' 'minutes' 'seconds'
# retry_count 		number of retries before scripts stops
#
# use_ipv6		detecting/sending IPv6 address
# force_ipversion	force usage of IPv4 or IPv6 for the whole detection and update communication
# dns_server		using a non default dns server to get Registered IP from Internet
# force_dnstcp		force communication with DNS server via TCP instead of default UDP
# proxy			using a proxy for communication !!! ALSO used to detect local IP via web => return proxy's IP !!!
# use_logfile		self-explanatory "/var/log/ddns/$SECTION_ID.log"
# is_glue			the record that should be updated is a glue record
#
# some functionality needs
# - GNU Wget or cURL installed for sending updates to DDNS service
# - BIND host installed to detect Registered IP
#
################################################################################

load_all_config_options "ddns" "$SECTION_ID"
ERR_LAST=$?	# save return code - equal 0 if SECTION_ID found

# set defaults if not defined
[ -z "$enabled" ]	  && enabled=0
[ -z "$retry_count" ]	  && retry_count=0	# endless retry
[ -z "$use_syslog" ]      && use_syslog=2	# syslog "Notice"
[ -z "$use_https" ]       && use_https=0	# not use https
[ -z "$use_logfile" ]     && use_logfile=1	# use logfile by default
[ -z "$use_ipv6" ]	  && use_ipv6=0		# use IPv4 by default
[ -z "$force_ipversion" ] && force_ipversion=0	# default let system decide
[ -z "$force_dnstcp" ]	  && force_dnstcp=0	# default UDP
[ -z "$ip_source" ]	  && ip_source="network"
[ -z "$is_glue" ]	  && is_glue=0		# default the ddns record is not a glue record
[ "$ip_source" = "network" -a -z "$ip_network" -a $use_ipv6 -eq 0 ] && ip_network="wan"  # IPv4: default wan
[ "$ip_source" = "network" -a -z "$ip_network" -a $use_ipv6 -eq 1 ] && ip_network="wan6" # IPv6: default wan6
[ "$ip_source" = "web" -a -z "$ip_url" -a $use_ipv6 -eq 0 ] && ip_url="http://checkip.dyndns.com"
[ "$ip_source" = "web" -a -z "$ip_url" -a $use_ipv6 -eq 1 ] && ip_url="http://checkipv6.dyndns.com"
[ "$ip_source" = "interface" -a -z "$ip_interface" ] && ip_interface="eth1"

# url encode username (might be email or something like this)
# and password (might have special chars for security reason)
# and optional parameter "param_enc"
[ -n "$username" ] && urlencode URL_USER "$username"
[ -n "$password" ] && urlencode URL_PASS "$password"
[ -n "$param_enc" ] && urlencode URL_PENC "$param_enc"

# SECTION_ID does not exists
[ $ERR_LAST -ne 0 ] && {
	[ $VERBOSE -le 1 ] && VERBOSE=2		# force console out and logfile output
	[ -f $LOGFILE ] && rm -f $LOGFILE	# clear logfile before first entry
	write_log  7 "************ ************** ************** **************"
	write_log  5 "PID '$$' started at $(eval $DATE_PROG)"
	write_log  7 "ddns version  : $VERSION"
	write_log  7 "uci configuration:${N}$(uci -q show ddns | grep '=service' | sort)"
	write_log 14 "Service section '$SECTION_ID' not defined"
}

write_log 7 "************ ************** ************** **************"
write_log 5 "PID '$$' started at $(eval $DATE_PROG)"
write_log 7 "ddns version  : $VERSION"
write_log 7 "uci configuration:${N}$(uci -q show ddns.$SECTION_ID | sort)"
# write_log 7 "ddns version  : $(opkg list-installed ddns-scripts | cut -d ' ' -f 3)"
case $VERBOSE in
	0) write_log  7 "verbose mode  : 0 - run normal, NO console output";;
	1) write_log  7 "verbose mode  : 1 - run normal, console mode";;
	2) write_log  7 "verbose mode  : 2 - run once, NO retry on error";;
	3) write_log  7 "verbose mode  : 3 - run once, NO retry on error, NOT sending update";;
	*) write_log 14 "error detecting VERBOSE '$VERBOSE'";;
esac

# check enabled state otherwise we don't need to continue
[ $enabled -eq 0 ] && write_log 14 "Service section disabled!"

# determine what update url we're using if a service_name is supplied
# otherwise update_url is set inside configuration (custom update url)
# or update_script is set inside configuration (custom update script)
[ -n "$service_name" ] && get_service_data update_url update_script UPD_ANSWER
[ -z "$update_url" -a -z "$update_script" ] && write_log 14 "No update_url found/defined or no update_script found/defined!"
[ -n "$update_script" -a ! -f "$update_script" ] && write_log 14 "Custom update_script not found!"

# temporary needed to convert existing uci settings
[ -z "$lookup_host" ] && {
	uci -q set ddns.$SECTION_ID.lookup_host="$domain"
	uci -q commit ddns
	lookup_host="$domain"
}
# later versions only check if configured correctly

# without lookup host and possibly other required options we can do nothing for you
[ -z "$lookup_host" ] && write_log 14 "Service section not configured correctly! Missing 'lookup_host'"

[ -n "$update_url" ] && {
	# only check if update_url is given, update_scripts have to check themselves
	[ -z "$domain" ] && $(echo "$update_url" | grep "\[DOMAIN\]" >/dev/null 2>&1) && \
		write_log 14 "Service section not configured correctly! Missing 'domain'"
	[ -z "$username" ] && $(echo "$update_url" | grep "\[USERNAME\]" >/dev/null 2>&1) && \
		write_log 14 "Service section not configured correctly! Missing 'username'"
	[ -z "$password" ] && $(echo "$update_url" | grep "\[PASSWORD\]" >/dev/null 2>&1) && \
		write_log 14 "Service section not configured correctly! Missing 'password'"
	[ -z "$param_enc" ] && $(echo "$update_url" | grep "\[PARAMENC\]" >/dev/null 2>&1) && \
		write_log 14 "Service section not configured correctly! Missing 'param_enc'"
	[ -z "$param_opt" ] && $(echo "$update_url" | grep "\[PARAMOPT\]" >/dev/null 2>&1) && \
		write_log 14 "Service section not configured correctly! Missing 'param_opt'"
}

# verify ip_source 'script' if script is configured and executable
if [ "$ip_source" = "script" ]; then
	set -- $ip_script	#handling script with parameters, we need a trick
	[ -z "$1" ] && write_log 14 "No script defined to detect local IP!"
	[ -x "$1" ] || write_log 14 "Script to detect local IP not executable!"
fi

# compute update interval in seconds
get_seconds CHECK_SECONDS ${check_interval:-10} ${check_unit:-"minutes"} # default 10 min
get_seconds FORCE_SECONDS ${force_interval:-72} ${force_unit:-"hours"}	 # default 3 days
get_seconds RETRY_SECONDS ${retry_interval:-60} ${retry_unit:-"seconds"} # default 60 sec
[ $CHECK_SECONDS -lt 300 ] && CHECK_SECONDS=300		# minimum 5 minutes
[ $FORCE_SECONDS -gt 0 -a $FORCE_SECONDS -lt $CHECK_SECONDS ] && FORCE_SECONDS=$CHECK_SECONDS	# FORCE_SECONDS >= CHECK_SECONDS or 0
write_log 7 "check interval: $CHECK_SECONDS seconds"
write_log 7 "force interval: $FORCE_SECONDS seconds"
write_log 7 "retry interval: $RETRY_SECONDS seconds"
write_log 7 "retry counter : $retry_count times"

# kill old process if it exists & set new pid file
stop_section_processes "$SECTION_ID"
[ $? -gt 0 ] && write_log 7 "'SIGTERM' was send to old process" || write_log 7 "No old process"
echo $$ > $PIDFILE

# determine when the last update was
# the following lines should prevent multiple updates if hotplug fires multiple startups
# as described in Ticket #7820, but did not function if never an update take place
# i.e. after a reboot (/var is linked to /tmp)
# using uptime as reference because date might not be updated via NTP client
get_uptime CURR_TIME
[ -e "$UPDFILE" ] && {
	LAST_TIME=$(cat $UPDFILE)
	# check also LAST > CURR because link of /var/run to /tmp might be removed
	# i.e. boxes with larger filesystems
	[ -z "$LAST_TIME" ] && LAST_TIME=0
	[ $LAST_TIME -gt $CURR_TIME ] && LAST_TIME=0
}
if [ $LAST_TIME -eq 0 ]; then
	write_log 7 "last update: never"
else
	EPOCH_TIME=$(( $(date +%s) - $CURR_TIME + $LAST_TIME ))
	EPOCH_TIME="date -d @$EPOCH_TIME +'$ddns_dateformat'"
	write_log 7 "last update: $(eval $EPOCH_TIME)"
fi

# verify DNS server
[ -n "$dns_server" ] && verify_dns "$dns_server"

# verify Proxy server and set environment
[ -n "$proxy" ] && {
	verify_proxy "$proxy" && {
		# everything ok set proxy
		export HTTP_PROXY="http://$proxy"
		export HTTPS_PROXY="http://$proxy"
		export http_proxy="http://$proxy"
		export https_proxy="http://$proxy"
	}
}

# let's check if there is already an IP registered on the web
get_registered_ip REGISTERED_IP "NO_RETRY"
ERR_LAST=$?
#     No error    or     No IP set	 otherwise retry
[ $ERR_LAST -eq 0 -o $ERR_LAST -eq 127 ] || get_registered_ip REGISTERED_IP
# on IPv6 we use expanded version to be shure when comparing
[ $use_ipv6 -eq 1 ] && expand_ipv6 "$REGISTERED_IP" REGISTERED_IP

# loop endlessly, checking ip every check_interval and forcing an updating once every force_interval
write_log 6 "Starting main loop at $(eval $DATE_PROG)"
while : ; do

	get_local_ip LOCAL_IP		# read local IP
	[ $use_ipv6 -eq 1 ] && expand_ipv6 "$LOCAL_IP" LOCAL_IP	# on IPv6 we use expanded version

	# prepare update
	# never updated or forced immediate then NEXT_TIME = 0
	[ $FORCE_SECONDS -eq 0 -o $LAST_TIME -eq 0 ] \
		&& NEXT_TIME=0 \
		|| NEXT_TIME=$(( $LAST_TIME + $FORCE_SECONDS ))

	get_uptime CURR_TIME		# get current uptime

	# send update when current time > next time or local ip different from registered ip
	if [ $CURR_TIME -ge $NEXT_TIME -o "$LOCAL_IP" != "$REGISTERED_IP" ]; then
		if [ $VERBOSE -gt 2 ]; then
			write_log 7 "Verbose Mode: $VERBOSE - NO UPDATE send"
		elif [ "$LOCAL_IP" != "$REGISTERED_IP" ]; then
			write_log 7 "Update needed - L: '$LOCAL_IP' <> R: '$REGISTERED_IP'"
		else
			write_log 7 "Forced Update - L: '$LOCAL_IP' == R: '$REGISTERED_IP'"
		fi

		ERR_LAST=0
		[ $VERBOSE -lt 3 ] && {
			# only send if VERBOSE < 3
			send_update "$LOCAL_IP"
			ERR_LAST=$?	# save return value
		}

		# error sending local IP to provider
		# we have no communication error (handled inside send_update/do_transfer)
		# but update was not recognized
		# do NOT retry after RETRY_SECONDS, do retry after CHECK_SECONDS
		# to early retrys will block most DDNS provider
		# providers answer is checked inside send_update() function
		if [ $ERR_LAST -eq 0 ]; then
			get_uptime LAST_TIME		# we send update, so
			echo $LAST_TIME > $UPDFILE	# save LASTTIME to file
			[ "$LOCAL_IP" != "$REGISTERED_IP" ] \
				&& write_log 6 "Update successful - IP '$LOCAL_IP' send" \
				|| write_log 6 "Forced update successful - IP: '$LOCAL_IP' send"
		elif [ $ERR_LAST -eq 127 ]; then
			write_log 3 "No update send to DDNS Provider"
		else
			write_log 3 "IP update not accepted by DDNS Provider"
		fi
	fi

	# now we wait for check interval before testing if update was recognized
	# only sleep if VERBOSE <= 2 because otherwise nothing was send
	[ $VERBOSE -le 2 ] && {
		write_log 7 "Waiting $CHECK_SECONDS seconds (Check Interval)"
		sleep $CHECK_SECONDS &
		PID_SLEEP=$!
		wait $PID_SLEEP	# enable trap-handler
		PID_SLEEP=0
	} || write_log 7 "Verbose Mode: $VERBOSE - NO Check Interval waiting"

	REGISTERED_IP=""		# clear variable
	get_registered_ip REGISTERED_IP	# get registered/public IP
	[ $use_ipv6 -eq 1 ] && expand_ipv6 "$REGISTERED_IP" REGISTERED_IP	# on IPv6 we use expanded version

	# IP's are still different
	if [ "$LOCAL_IP" != "$REGISTERED_IP" ]; then
		if [ $VERBOSE -le 1 ]; then	# VERBOSE <=1 then retry
			ERR_UPDATE=$(( $ERR_UPDATE + 1 ))
			[ $retry_count -gt 0 -a $ERR_UPDATE -gt $retry_count ] && \
				write_log 14 "Updating IP at DDNS provider failed after $retry_count retries"
			write_log 4 "Updating IP at DDNS provider failed - starting retry $ERR_UPDATE/$retry_count"
			continue # loop to beginning
		else
			write_log 4 "Updating IP at DDNS provider failed"
			write_log 7 "Verbose Mode: $VERBOSE - NO retry"; exit 1
		fi
	else
		# we checked successful the last update
		ERR_UPDATE=0			# reset error counter
	fi

	# force_update=0 or VERBOSE > 1 - leave here
	[ $VERBOSE -gt 1 ]  && write_log 7 "Verbose Mode: $VERBOSE - NO reloop"
	[ $FORCE_SECONDS -eq 0 ] && write_log 6 "Configured to run once"
	[ $VERBOSE -gt 1 -o $FORCE_SECONDS -eq 0 ] && exit 0

	write_log 6 "Rerun IP check at $(eval $DATE_PROG)"
done
# we should never come here there must be a programming error
write_log 12 "Error in 'dynamic_dns_updater.sh - program coding error"
