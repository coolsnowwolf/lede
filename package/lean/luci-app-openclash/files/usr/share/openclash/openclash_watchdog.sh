#!/bin/sh

CLASH="/etc/openclash/clash"
CLASH_CONFIG="/etc/openclash"
LOG_FILE="/tmp/openclash.log"
PROXY_FWMARK="0x162"
PROXY_ROUTE_TABLE="0x162"
enable_redirect_dns=$(uci get openclash.config.enable_redirect_dns 2>/dev/null)
dns_port=$(uci get openclash.config.dns_port 2>/dev/null)
disable_masq_cache=$(uci get openclash.config.disable_masq_cache 2>/dev/null)
en_mode=$(uci get openclash.config.en_mode 2>/dev/null)
cfg_update_interval=$(uci get openclash.config.config_update_interval 2>/dev/null)
CRASH_NUM=0
CFG_UPDATE_INT=0

if [ "$en_mode" = "fake-ip-tun" ] || [ "$en_mode" = "redir-host-tun" ] || [ "$en_mode" = "redir-host-mix" ] || [ "$en_mode" = "fake-ip-mix" ]; then
   core_type="Tun"
fi

if [ "$en_mode" = "redir-host-vpn" ] || [ "$en_mode" = "fake-ip-vpn" ]; then
   core_type="Game"
fi

while :;
do
   LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
   cfg_update=$(uci get openclash.config.auto_update 2>/dev/null)
   cfg_update_mode=$(uci get openclash.config.config_auto_update_mode 2>/dev/null)
   cfg_update_interval_now=$(uci get openclash.config.config_update_interval 2>/dev/null)
   enable=$(uci get openclash.config.enable)

if [ "$enable" -eq 1 ]; then
	if ! pidof clash >/dev/null; then
		 CRASH_NUM=$(expr "$CRASH_NUM" + 1)
		 if [ "$CRASH_NUM" -le 3 ]; then
	      CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
	      echo "${LOGTIME} Watchdog: Clash Core Problem, Restart." >> $LOG_FILE
	      nohup "$CLASH" -d "$CLASH_CONFIG" -f "$CONFIG_FILE" >> $LOG_FILE 2>&1 &
	      sleep 3
	      if [ "$core_type" = "Tun" ]; then
	         ip route replace default dev utun table "$PROXY_ROUTE_TABLE" 2>/dev/null
	         ip rule add fwmark "$PROXY_FWMARK" table "$PROXY_ROUTE_TABLE" 2>/dev/null
	      elif [ "$core_type" = "Game" ]; then
	         ip tuntap add user root mode tun clash0 2>/dev/null
           ip link set clash0 up 2>/dev/null
           ip route replace default dev clash0 table "$PROXY_ROUTE_TABLE" 2>/dev/null
           ip rule add fwmark "$PROXY_FWMARK" table "$PROXY_ROUTE_TABLE" 2>/dev/null
	      fi
	      /usr/share/openclash/openclash_history_set.sh
	   else
	      echo "${LOGTIME} Watchdog: Already Restart 3 Times With Clash Core Problem, Auto-Exit." >> $LOG_FILE
	      /etc/init.d/openclash stop
	      exit 0
	   fi
	else
	   CRASH_NUM=0
  fi
fi

## Porxy history
    /usr/share/openclash/openclash_history_get.sh

## Log File Size Manage:
    LOGSIZE=`ls -l /tmp/openclash.log |awk '{print int($5/1024)}'`
    if [ "$LOGSIZE" -gt 90 ]; then 
       echo "$LOGTIME Watchdog: Size Limit, Clean Up All Log Records." > $LOG_FILE
    fi

## 端口转发重启
   last_line=$(iptables -t nat -nL PREROUTING --line-number |awk '{print $1}' 2>/dev/null |awk 'END {print}' |sed -n '$p')
   op_line=$(iptables -t nat -nL PREROUTING --line-number |grep "openclash" 2>/dev/null |awk '{print $1}' 2>/dev/null |head -1)
   if [ "$last_line" != "$op_line" ] && [ -z "$core_type" ]; then
      iptables -t nat -D PREROUTING -p tcp -j openclash
      iptables -t nat -A PREROUTING -p tcp -j openclash
      echo "$LOGTIME Watchdog: Reset Firewall For Enabling Redirect." >>$LOG_FILE
   fi
   
## DNS转发劫持
   if [ "$enable_redirect_dns" -ne 0 ]; then
      if [ -z "$(uci get dhcp.@dnsmasq[0].server 2>/dev/null |grep "$dns_port")" ] || [ ! -z "$(uci get dhcp.@dnsmasq[0].server 2>/dev/null |awk -F ' ' '{print $2}')" ]; then
         echo "$LOGTIME Watchdog: Force Reset DNS Hijack." >> $LOG_FILE
         uci del dhcp.@dnsmasq[-1].server >/dev/null 2>&1
         uci add_list dhcp.@dnsmasq[0].server=127.0.0.1#"$dns_port"
         uci delete dhcp.@dnsmasq[0].resolvfile
         uci set dhcp.@dnsmasq[0].noresolv=1
         [ "$disable_masq_cache" -eq 1 ] && {
         	uci set dhcp.@dnsmasq[0].cachesize=0
         }
         uci commit dhcp
         /etc/init.d/dnsmasq restart >/dev/null 2>&1
      fi
   fi

## 配置文件循环更新
   if [ "$cfg_update" -eq 1 ] && [ "$cfg_update_mode" -eq 1 ]; then
      if [ -z "$cfg_update_interval_now" ]; then
         cfg_update_interval_now=60
      fi
      if [ -z "$cfg_update_interval" ]; then
         cfg_update_interval=60
      fi
      [ "$cfg_update_interval" -ne "$cfg_update_interval_now" ] && CFG_UPDATE_INT=0 && cfg_update_interval="$cfg_update_interval_now"
      if [ "$CFG_UPDATE_INT" -ne 0 ]; then
         [ "$(expr "$CFG_UPDATE_INT" % "$cfg_update_interval_now")" -eq 0 ] && /usr/share/openclash/openclash.sh
      fi
      CFG_UPDATE_INT=$(expr "$CFG_UPDATE_INT" + 1)
   fi

   sleep 60
done 2>/dev/null
