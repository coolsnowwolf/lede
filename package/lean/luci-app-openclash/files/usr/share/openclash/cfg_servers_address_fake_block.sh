#!/bin/sh

status=$(ps|grep -c /usr/share/openclash/cfg_servers_address_fake_block.sh)
[ "$status" -gt "3" ] && exit 0

en_mode=$(uci get openclash.config.en_mode 2>/dev/null)
if pidof clash >/dev/null && [ "$en_mode" != "redir-host" ]; then
   rm -rf /tmp/dnsmasq.d/dnsmasq_openclash.conf >/dev/null 2>&1
   /usr/share/openclash/openclash_fake_block.sh
   mkdir -p /tmp/dnsmasq.d
   ln -s /etc/openclash/dnsmasq_fake_block.conf /tmp/dnsmasq.d/dnsmasq_openclash.conf >/dev/null 2>&1
   /etc/init.d/dnsmasq restart >/dev/null 2>&1
fi
