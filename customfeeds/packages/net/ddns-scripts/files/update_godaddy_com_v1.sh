#!/bin/sh
#
#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
#
# script for sending updates to godaddy.com
#.based on GoDaddy.sh v1.0 by Nazar78 @ TeaNazaR.com
#.2017-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
# GoDaddy Documentation at https://developer.godaddy.com/doc
#
# This script is parsed by dynamic_dns_functions.sh inside send_update() function
#
# using following options from /etc/config/ddns
# option username - "key" as generated at https://developer.godaddy.com/keys/
# option password - "secret" as generated at https://developer.godaddy.com/keys/
# option domain   - "yourdomain.TLD" to update or "hostname@yourdomain.TLD"
#
# variable __IP already defined with the ip-address to use for update
#

# check parameters
[ -z "$CURL" ] && [ -z "$CURL_SSL" ] && write_log 14 "GoDaddy communication require cURL with SSL support. Please install"
[ -z "$username" ] && write_log 14 "Service section not configured correctly! Missing key as 'username'"
[ -z "$password" ] && write_log 14 "Service section not configured correctly! Missing secret as 'password'"
[ $use_https -eq 0 ] && use_https=1	# force HTTPS

# used variables
local __HOST __DOMAIN __TYPE __URL __PRGBASE __RUNPROG __DATA __IPV6

# split __HOST __DOMAIN from $domain
# given data:
# @example.com for "domain record"
# host.sub@example.com for a "host record"
__HOST=$(printf %s "$domain" | cut -d@ -f1)
__DOMAIN=$(printf %s "$domain" | cut -d@ -f2)

# GoDaddy needs:
# __DOMAIN = the base domain i.e. example.com
# __HOST   = host.sub if updating a host record or
# __HOST   = "@" urlencoded "%40" for a domain record
[ -z "$__HOST" -o "$__HOST" = "$__DOMAIN" ] && __HOST="%40"

# set record type
[ $use_ipv6 -eq 0 ] && __TYPE="A" || __TYPE="AAAA"

# now we know the url to use
# __URL="https://api.ote-godaddy.com/v1/domains/$__DOMAIN/records/$__TYPE/$__HOST"	# api test server
__URL="https://api.godaddy.com/v1/domains/$__DOMAIN/records/$__TYPE/$__HOST"		# production server

# transfer function to use for godaddy
# all needed variables are set global here
# so we can use them directly
godaddy_transfer() {
	local __CNT=0
	local __STATUS __ERR __DEVICE
	while : ; do
		write_log 7 "#> $__RUNPROG"
		__STATUS=$(eval "$__RUNPROG")
		__ERR=$?			# save communication error
		[ $__ERR -eq 0 ] && break	# no error break while

		write_log 4 "cURL error: '$__ERR'"
		write_log 7 "$(cat $ERRFILE)"		# report error

		[ $VERBOSE_MODE -gt 1 ] && {
			# VERBOSE_MODE > 1 then NO retry
			write_log 4 "Transfer failed - Verbose Mode: $VERBOSE_MODE - NO retry on error"
			break
		}

		__CNT=$(( $__CNT + 1 ))	# increment error counter
		# if error count > retry_count leave here
		[ $retry_count -gt 0 -a $__CNT -gt $retry_count ] && \
			write_log 14 "Transfer failed after $retry_count retries"

		write_log 4 "Transfer failed - retry $__CNT/$retry_count in $RETRY_SECONDS seconds"
		sleep $RETRY_SECONDS &
		PID_SLEEP=$!
		wait $PID_SLEEP	# enable trap-handler
		PID_SLEEP=0
	done

	# handle HTTP error
	[ $__STATUS -ne 200 ] && {
		write_log 4 "GoDaddy reported an error:"
		write_log 7 "$(cat $DATFILE)"
		return 1
	}
	return 0
}

# Build base command to use
__PRGBASE="$CURL -RsS -w '%{http_code}' -o $DATFILE --stderr $ERRFILE"
# force network/interface-device to use for communication
if [ -n "$bind_network" ]; then
	local __DEVICE
	network_get_physdev __DEVICE $bind_network || \
		write_log 13 "Can not detect local device using 'network_get_physdev $bind_network' - Error: '$?'"
	write_log 7 "Force communication via device '$__DEVICE'"
	__PRGBASE="$__PRGBASE --interface $__DEVICE"
fi
# force ip version to use
if [ $force_ipversion -eq 1 ]; then
	[ $use_ipv6 -eq 0 ] && __PRGBASE="$__PRGBASE -4" || __PRGBASE="$__PRGBASE -6"	# force IPv4/IPv6
fi
# set certificate parameters
if [ "$cacert" = "IGNORE" ]; then	# idea from Ticket #15327 to ignore server cert
	__PRGBASE="$__PRGBASE --insecure"	# but not empty better to use "IGNORE"
elif [ -f "$cacert" ]; then
	__PRGBASE="$__PRGBASE --cacert $cacert"
elif [ -d "$cacert" ]; then
	__PRGBASE="$__PRGBASE --capath $cacert"
elif [ -n "$cacert" ]; then		# it's not a file and not a directory but given
	write_log 14 "No valid certificate(s) found at '$cacert' for HTTPS communication"
fi
# disable proxy if no set (there might be .wgetrc or .curlrc or wrong environment set)
# or check if libcurl compiled with proxy support
if [ -z "$proxy" ]; then
	__PRGBASE="$__PRGBASE --noproxy '*'"
elif [ -z "$CURL_PROXY" ]; then
	# if libcurl has no proxy support and proxy should be used then force ERROR
	write_log 13 "cURL: libcurl compiled without Proxy support"
fi
# set headers
__PRGBASE="$__PRGBASE --header 'Authorization: sso-key $username:$password' "
__PRGBASE="$__PRGBASE --header 'Accept: application/json' "
__PRGBASE="$__PRGBASE --header 'Content-Type: application/json; charset=utf-8' "

# read data from godaddy.com
__RUNPROG="$__PRGBASE --request GET $__URL"
godaddy_transfer || return 1

# HTTP 200 OK, now analyse data and check if update needed
__DATA=$(sed -r 's/.+data":"(.+)","t.+/\1/g' $DATFILE)

# check data
[ $use_ipv6 -eq 0 ] \
	&& __DATA=$(printf "%s" "$__DATA" | grep -m 1 -o "$IPV4_REGEX") \
	|| __DATA=$(printf "%s" "$__DATA" | grep -m 1 -o "$IPV6_REGEX")

# we got data so verify
[ -n "$__DATA" ] && {
	# expand IPv6 for compare
	if [ $use_ipv6 -eq 1 ]; then
		expand_ipv6 $__IP __IPV6
		expand_ipv6 $__DATA __DATA
		[ "$__DATA" = "$__IPV6" ] && {		# IPv6 no update needed
			write_log 7 "IPv6 at GoDaddy.com already up to date"
			return 0
		}
	else
		[ "$__DATA" = "$__IP" ] && {		# IPv4 no update needed
			write_log 7 "IPv4 at GoDaddy.com already up to date"
			return 0
		}
	fi
}

# update is needed
# let's build data to send,
# use file to work around double quotes '"' needed for json
cat > $DATFILE << EOF
[{"data":"$__IP"}]
EOF

# let's complete transfer command
__RUNPROG="$__PRGBASE --request PUT --data @$DATFILE $__URL"

godaddy_transfer || return 1

# HTTP 200 OK
return 0

