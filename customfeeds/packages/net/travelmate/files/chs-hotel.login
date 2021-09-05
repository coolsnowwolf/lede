#!/bin/sh
# captive portal auto-login script for german chs hotels
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

trm_domain="hotspot.internet-for-guests.com"
trm_useragent="$(uci_get travelmate global trm_useragent "Mozilla/5.0 (Linux x86_64; rv:80.0) Gecko/20100101 Firefox/80.0")"
trm_maxwait="$(uci_get travelmate global trm_maxwait "30")"
trm_fetch="$(command -v curl)"

# initial get request to receive & extract valid security tokens
#
"${trm_fetch}" --user-agent "${trm_useragent}" --referer "http://www.example.com" --silent --connect-timeout $((trm_maxwait/6)) --cookie-jar "/tmp/${trm_domain}.cookie" --output /dev/null "https://${trm_domain}/logon/cgi/index.cgi"
if [ -r "/tmp/${trm_domain}.cookie" ]
then
	lg_id="$(awk '/LGNSID/{print $7}' "/tmp/${trm_domain}.cookie")"
	ta_id="$(awk '/ta_id/{print $7}' "/tmp/${trm_domain}.cookie")"
	cl_id="$(awk '/cl_id/{print $7}' "/tmp/${trm_domain}.cookie")"
	rm -f "/tmp/${trm_domain}.cookie"
else
	exit 2
fi

# final post request/login with valid session cookie/security token
#
if [ -n "${lg_id}" ] && [ -n "${ta_id}" ] && [ -n "${cl_id}" ]
then
	"${trm_fetch}" --user-agent "${trm_useragent}" --referer "https://${trm_domain}/logon/cgi/index.cgi" --silent --connect-timeout $((trm_maxwait/6)) --header "Cookie: LGNSID=${lg_id}; lang=en_US; use_mobile_interface=0; ta_id=${ta_id}; cl_id=${cl_id}" --data "accept_termsofuse=&freeperperiod=1&device_infos=1125:2048:1152:2048" --output /dev/null "https://${trm_domain}/logon/cgi/index.cgi"
else
	exit 3
fi
