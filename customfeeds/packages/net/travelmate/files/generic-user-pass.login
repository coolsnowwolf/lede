#!/bin/sh
# captive portal auto-login script template with credentials as parameters
# Copyright (c) 2020 Dirk Brenken (dev@brenken.org)
# This is free software, licensed under the GNU General Public License v3.

# set (s)hellcheck exceptions
# shellcheck disable=1091,2016,2039,2059,2086,2143,2181,2188

export LC_ALL=C
export PATH="/usr/sbin:/usr/bin:/sbin:/bin"
set -o pipefail

if [ "$(uci_get 2>/dev/null; printf "%u" "${?}")" = "127" ]
then
	. "/lib/functions.sh"
fi

trm_domain="example.com"
trm_useragent="$(uci_get travelmate global trm_useragent "Mozilla/5.0 (Linux x86_64; rv:80.0) Gecko/20100101 Firefox/80.0")"
trm_maxwait="$(uci_get travelmate global trm_maxwait "30")"
trm_fetch="$(command -v curl)"

user="${1}"
password="${2}"
success="Thank you!"

# login with credentials
#
response="$("${trm_fetch}" --user-agent "${trm_useragent}" --referer "http://www.example.com" --silent --connect-timeout $((trm_maxwait/6)) --data "username=${user}&password=${password}" --header "Content-Type:application/x-www-form-urlencoded" "http://${trm_domain}")"
if [ -n "$(printf "%s" "${response}" | grep "${success}")" ]
then
	exit 0
else
	exit 2
fi
