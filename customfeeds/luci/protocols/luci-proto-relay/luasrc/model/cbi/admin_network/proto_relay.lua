-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local ipaddr, network
local forward_bcast, forward_dhcp, gateway, expiry, retry, table


ipaddr = section:taboption("general", Value, "ipaddr",
	translate("Local IPv4 address"),
	translate("Address to access local relay bridge"))

ipaddr.datatype = "ip4addr"


network = s:taboption("general", DynamicList, "network", translate("Relay between networks"))
network.widget = "checkbox"
network.exclude = arg[1]
network.template = "cbi/network_netlist"
network.nocreate = true
network.nobridges = true
network.novirtual = true
network:depends("proto", "relay")


forward_bcast = section:taboption("advanced", Flag, "forward_bcast",
	translate("Forward broadcast traffic"))

forward_bcast.default = forward_bcast.enabled


forward_dhcp = section:taboption("advanced", Flag, "forward_dhcp",
	translate("Forward DHCP traffic"))

forward_dhcp.default = forward_dhcp.enabled


gateway = section:taboption("advanced", Value, "gateway",
	translate("Use DHCP gateway"),
	translate("Override the gateway in DHCP responses"))

gateway.datatype = "ip4addr"
gateway:depends("forward_dhcp", forward_dhcp.enabled)


expiry = section:taboption("advanced", Value, "expiry",
	translate("Host expiry timeout"),
	translate("Specifies the maximum amount of seconds after which hosts are presumed to be dead"))

expiry.placeholder = "30"
expiry.datatype    = "min(1)"


retry = section:taboption("advanced", Value, "retry",
	translate("ARP retry threshold"),
	translate("Specifies the maximum amount of failed ARP requests until hosts are presumed to be dead"))

retry.placeholder = "5"
retry.datatype    = "min(1)"


table = section:taboption("advanced", Value, "table",
	translate("Use routing table"),
	translate("Override the table used for internal routes"))

table.placeholder = "16800"
table.datatype    = "range(0,65535)"
