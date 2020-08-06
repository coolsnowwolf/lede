#!/bin/sh

status=$(ps|grep -c /usr/share/openclash/cfg_servers_address_fake_filter.sh)
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
en_mode=$(uci get openclash.config.en_mode 2>/dev/null)

if pidof clash >/dev/null && [ -z "$(echo "$en_mode" |grep "redir-host")" ]; then
   rm -rf /tmp/dnsmasq.d/dnsmasq_openclash.conf >/dev/null 2>&1
   /usr/share/openclash/openclash_fake_filter.sh
   if [ -s "/etc/openclash/servers_fake_filter.conf" ]; then
      mkdir -p /tmp/dnsmasq.d
      ln -s /etc/openclash/servers_fake_filter.conf /tmp/dnsmasq.d/dnsmasq_openclash.conf >/dev/null 2>&1
      /etc/init.d/dnsmasq restart >/dev/null 2>&1
   fi
   echo "" >$START_LOG
fi
