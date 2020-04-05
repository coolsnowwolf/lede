#!/bin/bash /etc/rc.common

START_LOG="/tmp/openclash_start.log"
echo "正在设置Fake-IP黑名单..." >$START_LOG

direct_dns=$(uci get openclash.config.direct_dns 2>/dev/null)
[ -z "$direct_dns" ] && {
	direct_dns="114.114.114.114"
	}
rm -rf /etc/openclash/dnsmasq_fake_block.conf 2>/dev/null
for i in `cat /etc/openclash/custom/openclash_custom_fake_black.conf`
do
   if [ -z "$(echo $i |grep '^ \{0,\}#' 2>/dev/null)" ]; then
      echo "server=/$i/$direct_dns" >>/etc/openclash/dnsmasq_fake_block.conf
	 fi
done

cfg_server_address()
{
	 local section="$1"
   config_get "server" "$section" "server" ""
   
   IFIP=$(echo $server |grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$")
   if [ -z "$IFIP" ] && [ ! -z "$server" ]; then
      echo "server=/$server/$direct_dns" >>/etc/openclash/dnsmasq_fake_block.conf
      noip="false"
   else
      return
   fi
}

#Fake下正确检测节点延迟
noip="true"
echo "#Server Nodes" >>/etc/openclash/dnsmasq_fake_block.conf
config_load "openclash"
config_foreach cfg_server_address "servers"
[ "$noip" = "true" ] && {
   sed -i '/#Server Nodes/d' /etc/openclash/dnsmasq_fake_block.conf 2>/dev/null
}
echo "" >$START_LOG
