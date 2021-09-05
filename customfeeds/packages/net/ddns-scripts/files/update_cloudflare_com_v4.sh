#!/bin/sh
#
#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
#
# script for sending updates to cloudflare.com
#.based on Ben Kulbertis cloudflare-update-record.sh found at http://gist.github.com/benkulbertis
#.and on George Johnson's cf-ddns.sh found at https://github.com/gstuartj/cf-ddns.sh
#.2016-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
# CloudFlare API documentation at https://api.cloudflare.com/
#
# This script is parsed by dynamic_dns_functions.sh inside send_update() function
#
# using following options from /etc/config/ddns
# option username  - your cloudflare e-mail
# option password  - cloudflare api key, you can get it from cloudflare.com/my-account/
# option domain    - "hostname@yourdomain.TLD"	# syntax changed to remove split_FQDN() function and tld_names.dat.gz
#
# The proxy status would not be changed by this script. Please change it in Cloudflare dashboard manually. 
#
# variable __IP already defined with the ip-address to use for update
#

# check parameters
[ -z "$CURL" ] && [ -z "$CURL_SSL" ] && write_log 14 "Cloudflare communication require cURL with SSL support. Please install"
[ -z "$username" ] && write_log 14 "Service section not configured correctly! Missing key as 'username'"
[ -z "$password" ] && write_log 14 "Service section not configured correctly! Missing secret as 'password'"
[ $use_https -eq 0 ] && use_https=1	# force HTTPS

# used variables
local __HOST __DOMAIN __TYPE __URLBASE __PRGBASE __RUNPROG __DATA __IPV6 __ZONEID __RECID __PROXIED
local __URLBASE="https://api.cloudflare.com/client/v4"
local __TTL=120

# split __HOST __DOMAIN from $domain
# given data:
# @example.com for "domain record"
# host.sub@example.com for a "host record"
__HOST=$(printf %s "$domain" | cut -d@ -f1)
__DOMAIN=$(printf %s "$domain" | cut -d@ -f2)

# Cloudflare v4 needs:
# __DOMAIN = the base domain i.e. example.com
# __HOST   = the FQDN of record to modify
# i.e. example.com for the "domain record" or host.sub.example.com for "host record"

# handling domain record then set __HOST = __DOMAIN
[ -z "$__HOST" ] && __HOST=$__DOMAIN
# handling host record then rebuild fqdn host@domain.tld => host.domain.tld
[ "$__HOST" != "$__DOMAIN" ] && __HOST="${__HOST}.${__DOMAIN}"

# set record type
[ $use_ipv6 -eq 0 ] && __TYPE="A" || __TYPE="AAAA"

# transfer function to use for godaddy
# all needed variables are set global here
# so we can use them directly
cloudflare_transfer() {
	local __CNT=0
	local __ERR
	while : ; do
		write_log 7 "#> $__RUNPROG"
		eval "$__RUNPROG"
		__ERR=$?			# save communication error
		[ $__ERR -eq 0 ] && break	# no error break while

		write_log 3 "cURL Error: '$__ERR'"
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

	# check for error
	grep -q '"success":\s*true' $DATFILE || {
		write_log 4 "CloudFlare reported an error:"
		write_log 7 "$(cat $DATFILE)"		# report error
		return 1	# HTTP-Fehler
	}
}

# Build base command to use
__PRGBASE="$CURL -RsS -o $DATFILE --stderr $ERRFILE"
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
# disable proxy if not set (there might be .wgetrc or .curlrc or wrong environment set)
# or check if libcurl compiled with proxy support
if [ -z "$proxy" ]; then
	__PRGBASE="$__PRGBASE --noproxy '*'"
elif [ -z "$CURL_PROXY" ]; then
	# if libcurl has no proxy support and proxy should be used then force ERROR
	write_log 13 "cURL: libcurl compiled without Proxy support"
fi
# set headers
if [ "$username" = "Bearer" ]; then
  write_log 7 "Found Username 'Bearer' using Password as Bearer Authorization Token"
  __PRGBASE="$__PRGBASE --header 'Authorization: Bearer $password' "
else
  __PRGBASE="$__PRGBASE --header 'X-Auth-Email: $username' "
  __PRGBASE="$__PRGBASE --header 'X-Auth-Key: $password' "
fi
__PRGBASE="$__PRGBASE --header 'Content-Type: application/json' "

# read zone id for registered domain.TLD
__RUNPROG="$__PRGBASE --request GET '$__URLBASE/zones?name=$__DOMAIN'"
cloudflare_transfer || return 1
# extract zone id
__ZONEID=$(grep -o '"id":\s*"[^"]*' $DATFILE | grep -o '[^"]*$' | head -1)
[ -z "$__ZONEID" ] && {
	write_log 4 "Could not detect 'zone id' for domain.tld: '$__DOMAIN'"
	return 127
}

# read record id for A or AAAA record of host.domain.TLD
__RUNPROG="$__PRGBASE --request GET '$__URLBASE/zones/$__ZONEID/dns_records?name=$__HOST&type=$__TYPE'"
cloudflare_transfer || return 1
# extract record id
__RECID=$(grep -o '"id":\s*"[^"]*' $DATFILE | grep -o '[^"]*$' | head -1)
[ -z "$__RECID" ] && {
	write_log 4 "Could not detect 'record id' for host.domain.tld: '$__HOST'"
	return 127
}

# extract current stored IP
__DATA=$(grep -o '"content":\s*"[^"]*' $DATFILE | grep -o '[^"]*$' | head -1)

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
			write_log 7 "IPv6 at CloudFlare.com already up to date"
			return 0
		}
	else
		[ "$__DATA" = "$__IP" ] && {		# IPv4 no update needed
			write_log 7 "IPv4 at CloudFlare.com already up to date"
			return 0
		}
	fi
}

# update is needed
# let's build data to send
# set proxied parameter
__PROXIED=$(grep -o '"proxied":\s*[^",]*' $DATFILE | grep -o '[^:]*$')

# use file to work around " needed for json
cat > $DATFILE << EOF
{"id":"$__ZONEID","type":"$__TYPE","name":"$__HOST","content":"$__IP","ttl":$__TTL,"proxied":$__PROXIED}
EOF

# let's complete transfer command
__RUNPROG="$__PRGBASE --request PUT --data @$DATFILE '$__URLBASE/zones/$__ZONEID/dns_records/$__RECID'"
cloudflare_transfer || return 1

return 0
