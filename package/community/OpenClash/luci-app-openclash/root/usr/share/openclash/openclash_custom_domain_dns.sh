#!/bin/sh
. /usr/share/openclash/log.sh

set_lock() {
   exec 883>"/tmp/lock/openclash_cus_domian.lock" 2>/dev/null
   flock -x 883 2>/dev/null
}

del_lock() {
   flock -u 883 2>/dev/null
   rm -rf "/tmp/lock/openclash_cus_domian.lock"
}

set_lock

rm -rf /tmp/dnsmasq.d/dnsmasq_openclash_custom_domain.conf >/dev/null 2>&1
if [ "$(uci get openclash.config.dns_advanced_setting 2>/dev/null)" -eq 1 ]; then
   LOG_OUT "Setting Secondary DNS Server List..."

   custom_domain_dns_server=$(uci get openclash.config.custom_domain_dns_server 2>/dev/null)
   [ -z "$custom_domain_dns_server" ] && {
	   custom_domain_dns_server="114.114.114.114"
	 }

   if [ -s "/etc/openclash/custom/openclash_custom_domain_dns.list" ]; then
      mkdir -p /tmp/dnsmasq.d
      awk -v tag="$custom_domain_dns_server" '!/^$/&&!/^#/{printf("server=/%s/"'tag'"\n",$0)}' /etc/openclash/custom/openclash_custom_domain_dns.list >>/tmp/dnsmasq.d/dnsmasq_openclash_custom_domain.conf 2>/dev/null
   fi
fi

del_lock