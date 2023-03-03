#!/bin/sh

uci set network.wan='interface'
uci set network.wan.ifname='wwan0'
uci set network.wan.proto='rmnet'

uci set network.wan6='interface'
uci set network.wan6.ifname='wwan0'
uci set network.wan6.proto='rmnet6'

uci set dhcp.lan.ra='relay'
uci set dhcp.lan.dhcpv6='disabled'
uci set dhcp.lan.ndp='relay'

uci set dhcp.wan.ra='relay'
uci set dhcp.wan.dhcpv6='disabled'
uci set dhcp.wan.ndp='relay'
uci set dhcp.wan.ndproxy_routing='0'

uci set dhcp.wan6=dhcp
uci set dhcp.wan6.interface='wan6'
uci set dhcp.wan6.ra='relay'
uci set dhcp.wan6.dhcpv6='disabled'
uci set dhcp.wan6.ndp='relay'
uci set dhcp.wan6.ndproxy_routing='0'
uci set dhcp.wan6.master='1'

uci set dhcp.odhcpd=odhcpd
uci set dhcp.odhcpd.loglevel='7'

uci commit
