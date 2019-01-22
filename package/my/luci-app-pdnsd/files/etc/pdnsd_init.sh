#!/bin/sh /etc/rc.common
restart(){
	echo luci for pdnsd
	sleep 1s
	local vt_enabled=`uci get pdnsd.@arguments[0].enabled 2>/dev/null`
	echo $vt_enabled 
	logger -t alex !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!pdnsd is initializing enabled is $vt_enabled

	if [ $vt_enabled = 1 ]; then
		logger -t alex restarting pdnsd
 		echo "restarting pdnsd"
		uci delete dhcp.@dnsmasq[0].server
		uci add_list dhcp.@dnsmasq[0].server=127.0.0.1#5053
		uci delete dhcp.@dnsmasq[0].resolvfile
		uci set dhcp.@dnsmasq[0].noresolv=1
		uci commit dhcp
		cp /etc/pdnsd_gfw.cfg /etc/pdnsd.conf
		/etc/init.d/dnsmasq restart
		/etc/init.d/pdnsd enable
		/etc/init.d/pdnsd restart

	else	
		logger -t alex stopping pdnsd
		echo "stopping pdnsd"
		/etc/init.d/pdnsd disable
		/etc/init.d/pdnsd stop 
		uci del_list dhcp.@dnsmasq[0].server=127.0.0.1#5053
		uci set dhcp.@dnsmasq[0].resolvfile=/tmp/resolv.conf.auto
		uci delete dhcp.@dnsmasq[0].noresolv
		uci commit dhcp
		/etc/init.d/dnsmasq restart 
	fi
}