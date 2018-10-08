#!/bin/sh

dns_enable=$(uci flowoffload.@flow[0].dns 2>/dev/null)

if [ $dns_enable -eq 1 ]; then
	if ! pidof dnscache>/dev/null; then
	   /etc/init.d/flowoffload restart
  fi
fi
