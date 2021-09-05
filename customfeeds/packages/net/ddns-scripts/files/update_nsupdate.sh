#
#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
#
# The script directly updates a PowerDNS (or maybe bind server) via nsupdate from bind-client package.
#.based on github request #957 by Jan Riechers <de at r-jan dot de>
#.2015 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
#
# This script is parsed by dynamic_dns_functions.sh inside send_update() function
#
# using following options from /etc/config/ddns
# option username   - keyname 
# option password   - shared secret (base64 encoded)
# option domain     - full qualified domain to update
# option dns_server - DNS server to update
#
# variable __IP already defined with the ip-address to use for update
#
local __TTL=600		#.preset DNS TTL (in seconds)
local __RRTYPE __PW __TCP
local __PROG=$(which nsupdate)			# BIND nsupdate ?
[ -z "$__PROG" ] && __PROG=$(which knsupdate)	# Knot nsupdate ?

[ -z "$__PROG" ]     && write_log 14 "'nsupdate' or 'knsupdate' not installed !"
[ -z "$username" ]   && write_log 14 "Service section not configured correctly! Missing 'username'"
[ -z "$password" ]   && write_log 14 "Service section not configured correctly! Missing 'password'"
[ -z "$dns_server" ] && write_log 14 "Service section not configured correctly! Missing 'dns_server'"

[ $use_ipv6 -ne 0 ] && __RRTYPE="AAAA" || __RRTYPE="A"
[ $force_dnstcp -ne 0 ] && __TCP="-v" || __TCP=""

# create command file
cat >$DATFILE <<-EOF
server $dns_server
key $username $password
update del $domain $__RRTYPE
update add $domain $__TTL $__RRTYPE $__IP
show
send
answer
quit
EOF

$__PROG -d $__TCP $DATFILE >$ERRFILE 2>&1

# nsupdate always return success
write_log 7 "(k)nsupdate reports:${N}$(cat $ERRFILE)"

return 0
