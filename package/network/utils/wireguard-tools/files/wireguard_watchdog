#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2018 Aleksandr V. Piskunov <aleksandr.v.piskunov@gmail.com>.
# Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
#
# This watchdog script tries to re-resolve hostnames for inactive WireGuard peers.
# Use it for peers with a frequently changing dynamic IP.
# persistent_keepalive must be set, recommended value is 25 seconds.
#
# Run this script from cron every minute:
# echo '* * * * * /usr/bin/wireguard_watchdog' >> /etc/crontabs/root


. /lib/functions.sh

check_peer_activity() {
  local cfg=$1
  local iface=$2
  local public_key
  local endpoint_host
  local endpoint_port
  local persistent_keepalive
  local last_handshake
  local idle_seconds

  config_get public_key "${cfg}" "public_key"
  config_get endpoint_host "${cfg}" "endpoint_host"
  config_get endpoint_port "${cfg}" "endpoint_port"
  persistent_keepalive=$(wg show ${iface} persistent-keepalive | grep ${public_key} | awk '{print $2}')

  # only process peers with endpoints and keepalive set
  [ -z ${endpoint_host} ] && return 0;
  [ -z ${persistent_keepalive} -o ${persistent_keepalive} = "off" ] && return 0;

  # skip IP addresses
  # check taken from packages/net/ddns-scripts/files/dynamic_dns_functions.sh
  local IPV4_REGEX="[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}"
  local IPV6_REGEX="\(\([0-9A-Fa-f]\{1,4\}:\)\{1,\}\)\(\([0-9A-Fa-f]\{1,4\}\)\{0,1\}\)\(\(:[0-9A-Fa-f]\{1,4\}\)\{1,\}\)"
  local IPV4=$(echo ${endpoint_host} | grep -m 1 -o "$IPV4_REGEX$")    # do not detect ip in 0.0.0.0.example.com
  local IPV6=$(echo ${endpoint_host} | grep -m 1 -o "$IPV6_REGEX")
  [ -n "${IPV4}" -o -n "${IPV6}" ] && return 0;

  # re-resolve endpoint hostname if not responding for too long
  last_handshake=$(wg show ${iface} latest-handshakes | grep ${public_key} | awk '{print $2}')
  [ -z ${last_handshake} ] && return 0;
  idle_seconds=$(($(date +%s)-${last_handshake}))
  [ ${idle_seconds} -lt 150 ] && return 0;
  logger -t "wireguard_monitor" "${iface} endpoint ${endpoint_host}:${endpoint_port} is not responding for ${idle_seconds} seconds, trying to re-resolve hostname"
  wg set ${iface} peer ${public_key} endpoint "${endpoint_host}:${endpoint_port}"
}

# query ubus for all active wireguard interfaces
wg_ifaces=$(ubus -S call network.interface dump | jsonfilter -e '@.interface[@.up=true]' | jsonfilter -a -e '@[@.proto="wireguard"].interface' | tr "\n" " ")

# check every peer in every active wireguard interface
config_load network
for iface in $wg_ifaces; do
  config_foreach check_peer_activity "wireguard_${iface}" "${iface}"
done
