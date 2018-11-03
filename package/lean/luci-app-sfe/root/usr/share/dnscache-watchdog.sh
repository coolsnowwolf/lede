#!/bin/sh

dns_enable=$(uci get sfe.config.dns 2>/dev/null)

if [ $dns_enable -eq 1 ]; then
	if ! pidof dnscache>/dev/null; then
	   /etc/init.d/sfe restart
  fi
fi
