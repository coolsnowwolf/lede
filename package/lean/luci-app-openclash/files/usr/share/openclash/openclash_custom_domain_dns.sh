#!/bin/sh

status=$(ps|grep -c /usr/share/openclash/openclash_custom_domain_dns.sh)
[ "$status" -gt "3" ] && exit 0

START_LOG="/tmp/openclash_start.log"
rm -rf /tmp/dnsmasq.d/dnsmasq_openclash_custom_domain.conf >/dev/null 2>&1
if [ "$(uci get openclash.config.dns_advanced_setting 2>/dev/null)" -eq 1 ]; then
   echo "正在设置第二DNS服务器列表..." >$START_LOG

   custom_domain_dns_server=$(uci get openclash.config.custom_domain_dns_server 2>/dev/null)
   [ -z "$custom_domain_dns_server" ] && {
	   custom_domain_dns_server="114.114.114.114"
	 }

   if [ -s "/etc/openclash/custom/openclash_custom_domain_dns.list" ]; then
      mkdir -p /tmp/dnsmasq.d
      awk -v tag="$custom_domain_dns_server" '!/^$/&&!/^#/{printf("server=/%s/"'tag'"\n",$0)}' /etc/openclash/custom/openclash_custom_domain_dns.list >>/tmp/dnsmasq.d/dnsmasq_openclash_custom_domain.conf 2>/dev/null
   fi
fi