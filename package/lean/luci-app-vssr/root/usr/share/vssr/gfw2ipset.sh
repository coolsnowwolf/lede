#!/bin/sh
. /lib/functions.sh

uci_get_by_type() {
    local ret=$(uci get vssr.@$1[0].$2 2>/dev/null)
    echo ${ret:=$3}
}
v2ray_flow=$(uci_get_by_type global v2ray_flow)

mkdir -p /tmp/dnsmasq.ssr

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"gfwlist"'\n",$0)}' /etc/vssr/gfw.list >/tmp/dnsmasq.ssr/custom_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/gfw.list >>/tmp/dnsmasq.ssr/custom_forward.conf

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/black.list >/tmp/dnsmasq.ssr/blacklist_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/black.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"whitelist"'\n",$0)}' /etc/vssr/white.list >/tmp/dnsmasq.ssr/whitelist_forward.conf

if [ "$v2ray_flow" = "1" ]; then

    awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/tw_video_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/tw_video_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    
    awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/netflix_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/netflix_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    
    awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/disney_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/disney_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    
    awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/prime_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/prime_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    
    awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/tvb_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/tvb_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    
    awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/vssr/custom_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
    awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/vssr/custom_domain.list >>/tmp/dnsmasq.ssr/blacklist_forward.conf
fi

function valid_ip() {
  ip=$1
  read_ip=$(echo $ip | awk -F. '$1<=255&&$2<=255&&$3<=255&&$4<=255{print "yes"}')
  if echo $ip | grep -E '^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$' >/dev/null; then
    if [ $read_ip == "yes" ]; then
      return 0
    else
      return 1
    fi
  else
    return 1
  fi
}

config_load vssr

function addWhiteList() {
  local iface="$1"
  local host
  config_get host "$iface" server
  if valid_ip $host; then
    ipset -! add whitelist $host
  else
    [ ! -z "$host" ] && echo "ipset=/.$host/whitelist" >>/tmp/dnsmasq.ssr/whitelist_forward.conf
  fi
}

config_foreach addWhiteList
