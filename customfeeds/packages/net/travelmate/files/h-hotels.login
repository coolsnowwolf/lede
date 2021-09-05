#!/bin/sh
# captive portal auto-login script for Telekom hotspots in german h+hotels
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

trm_domain="hotspot.t-mobile.net"
trm_useragent="$(uci_get travelmate global trm_useragent "Mozilla/5.0 (Linux x86_64; rv:80.0) Gecko/20100101 Firefox/80.0")"
trm_maxwait="$(uci_get travelmate global trm_maxwait "30")"
trm_fetch="$(command -v curl)"

# initial get request to receive & extract valid security tokens
#
"${trm_fetch}" --user-agent "${trm_useragent}" --referer "http://www.example.com" --silent --connect-timeout $((trm_maxwait/6)) --cookie-jar "/tmp/${trm_domain}.cookie" --output /dev/null "https://${trm_domain}/wlan/rest/freeLogin"
if [ -r "/tmp/${trm_domain}.cookie" ]
then
	ses_id="$(awk '/JSESSIONID/{print $7}' "/tmp/${trm_domain}.cookie")"
	sec_id="$(awk '/DT_H/{print $7}' "/tmp/${trm_domain}.cookie")"
	dev_id="$(sha256sum /etc/config/wireless | awk '{printf "%s",substr($1,1,13)}')"
	rm -f "/tmp/${trm_domain}.cookie"
else
	exit 2
fi

# final post request/login with valid session cookie/security token
#
if [ -n "${ses_id}" ] && [ -n "${sec_id}" ] && [ -n "${dev_id}" ]
then
	"${trm_fetch}" --user-agent "${trm_useragent}" --referer "https://${trm_domain}/TD/hotspot/H_Hotels/en_GB/index.html" --silent --connect-timeout $((trm_maxwait/6)) --header "Cookie: JSESSIONID=${ses_id}; DT_DEV_ID=${dev_id}; DT_H=${sec_id}" --data "rememberMe=true" --output /dev/null "https://${trm_domain}/wlan/rest/freeLogin"
else
	exit 3
fi
