#!/bin/sh

mkdir -p /tmp/dnsmasq.ssr

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"gfwlist"'\n",$0)}' /etc/config/gfw.list > /tmp/dnsmasq.ssr/custom_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/config/gfw.list >> /tmp/dnsmasq.ssr/custom_forward.conf

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/config/black.list > /tmp/dnsmasq.ssr/blacklist_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/config/black.list >> /tmp/dnsmasq.ssr/blacklist_forward.conf

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"whitelist"'\n",$0)}' /etc/config/white.list > /tmp/dnsmasq.ssr/whitelist_forward.conf

