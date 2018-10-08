#!/bin/sh

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"gfwlist"'\n",$0)}' /etc/config/gfw.list > /etc/dnsmasq.ssr/custom_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5353"'\n",$0)}' /etc/config/gfw.list >> /etc/dnsmasq.ssr/custom_forward.conf

