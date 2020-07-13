--[[

Copyright (C) 2018 TDT AG <development@tdt.de>

This is free software, licensed under the Apache License Version 2.0.
See https://www.apache.org/licenses/LICENSE-2.0 for more information.

]]--

local map, section, net = ...

local ipaddr, netmask, gateway, broadcast

local SYS = require "luci.sys"

local dhcp, slaves, bonding_policy, primary, primary_reselect, min_links, ad_actor_sys_prio, ad_actor_system
local ad_select, lacp_rate, packets_per_slave, lp_interval, tlb_dynamic_lb, fail_over_mac
local num_grat_arp__num_unsol_na, xmit_hash_policy, resend_igmp, all_slaves_active, link_monitoring
local arp_interval, arp_ip_target, arp_all_targets, arp_validate, miimon, downdelay, updelay, use_carrier

local function get_selectable_slaves(field)
	m.uci:foreach("network", "interface",
		function (section)
			if section[".name"] ~= "loopback" then
				local network_section_ifname = m.uci:get("network", section['.name'], "ifname")
				local in_use = false

				m.uci:foreach("network", "interface",
					function (section)
						if m.uci:get("network", section['.name'], "proto") == "bonding" then

							local bonding_section_slaves = m.uci:get("network", section['.name'], "slaves")

							if bonding_section_slaves ~= nil then

								for this_slave in bonding_section_slaves:gmatch("[%S-]+") do

									if network_section_ifname == this_slave:gsub("-", "") and section['.name'] ~= arg[1] then
										in_use = true
									end
								end
							end
						end
					end
					)

				if in_use == false and network_section_ifname ~= nil then
					if network_section_ifname:find("eth") ~= nil then
						field:value(network_section_ifname, network_section_ifname)
					end
				end
			end
		end
	)
end


local function get_selectable_slaves_from_proc(field)
	local interfaces = SYS.exec("cat /proc/net/dev | grep 'eth' | awk '\{print \$1\}' | tr ':\n' ' '")

	if interfaces ~= nil then
		for this_interface in interfaces:gmatch("[%S-]+") do
			local in_use = false

			m.uci:foreach("network", "interface",
				function (section)
					if m.uci:get("network", section['.name'], "proto") == "bonding" then
						local bonding_section_slaves = m.uci:get("network", section['.name'], "slaves")

						if bonding_section_slaves ~= nil then

							for this_slave in bonding_section_slaves:gmatch("[%S-]+") do

								if this_interface == this_slave and section['.name'] ~= arg[1] then
									in_use = true
								end
							end
						end
					end
				end	
				)

			if in_use == false then
				field:value(this_interface, this_interface)
			end
		end
	end
end


ipaddr = section:taboption("general", Value, "ipaddr",
	translate("IPv4 address"))
ipaddr.datatype = "ip4addr"
ipaddr.optional = false
ipaddr.rmempty = false


netmask = section:taboption("general", Value, "netmask",
	translate("IPv4 netmask"))
netmask.datatype = "ip4addr"
netmask.optional = false
netmask.rmempty = false
netmask:value("255.255.255.0")
netmask:value("255.255.0.0")
netmask:value("255.0.0.0")


-- dhcp = section:taboption("general", Flag, "dhcp",
-- 	translate("Use DHCP (Client Mode)"),
-- 	translate("Specifies whether the bonding interface should use DHCP client mode"))
-- dhcp.default = dhcp.disabled
-- dhcp.rmempty = false

slaves = section:taboption("advanced", MultiValue, "slaves",
        translate ("Slave Interfaces"),
        translate("Specifies which slave interfaces should be attached to this bonding interface"))
slaves.oneline = true
slaves.widget = "checkbox"

get_selectable_slaves_from_proc(slaves)

bonding_policy = section:taboption("advanced", ListValue, "bonding_policy",
	translate("Bonding Policy"),
	translate("Specifies the mode to be used for this bonding interface"))
bonding_policy.default = "balance-rr"
bonding_policy:value("balance-rr", translate("Round-Robin policy (balance-rr, 0)"))
bonding_policy:value("active-backup", translate("Active-Backup policy (active-backup, 1)"))
bonding_policy:value("balance-xor", translate("XOR policy (balance-xor, 2)"))
bonding_policy:value("broadcast", translate("Broadcast policy (broadcast, 3)"))
bonding_policy:value("802.3ad", translate("IEEE 802.3ad Dynamic link aggregation (802.3ad, 4)"))
bonding_policy:value("balance-tlb", translate("Adaptive transmit load balancing (balance-tlb, 5)"))
bonding_policy:value("balance-alb", translate("Adaptive load balancing (balance-alb, 6)"))

primary = section:taboption("advanced", ListValue, "primary",
	translate("Primary Slave"),
	translate("Specifies which slave is the primary device. It will always be the active slave while it is available"))
primary.widget = "radio"
primary.orientation = "horizontal"
primary:depends("bonding_policy", "active-backup")
primary:depends("bonding_policy", "balance-tlb")
primary:depends("bonding_policy", "balance-alb")

get_selectable_slaves_from_proc(primary)

primary_reselect = section:taboption("advanced", ListValue, "primary_reselect",
	translate("Reselection policy for primary slave"),
	translate("Specifies the reselection policy for the primary slave when failure of the active slave or recovery of the primary slave occurs"))
primary_reselect.default = "always"
primary_reselect:value("always", translate("Primary becomes active slave whenever it comes back up (always, 0)"))
primary_reselect:value("better", translate("Primary becomes active slave when it comes back up if speed and duplex better than current slave (better, 1)"))
primary_reselect:value("failure", translate("Only if current active slave fails and the primary slave is up (failure, 2)"))
primary_reselect:depends("bonding_policy", "active-backup")
primary_reselect:depends("bonding_policy", "balance-tlb")
primary_reselect:depends("bonding_policy", "balance-alb")

min_links = section:taboption("advanced", Value, "min_links",
	translate("Minimum Number of Links"),
	translate("Specifies the minimum number of links that must be active before asserting carrier"))
min_links.datatype = "uinteger"
min_links.default = 0
min_links:depends("bonding_policy", "802.3ad")

ad_actor_sys_prio = section:taboption("advanced", Value, "ad_actor_sys_prio",
	translate("System Priority"),
	translate("Specifies the system priority"))
ad_actor_sys_prio.datatype = "range(1,65535)"
ad_actor_sys_prio.default = 65535
ad_actor_sys_prio:depends("bonding_policy", "802.3ad")

ad_actor_system = section:taboption("advanced", Value, "ad_actor_system",
	translate("MAC Address For The Actor"),
	translate("Specifies the mac-address for the actor in protocol packet exchanges (LACPDUs). If empty, masters' mac address defaults to system default"))
ad_actor_system.datatype = "macaddr"
ad_actor_system.default = ""
ad_actor_system:depends("bonding_policy", "802.3ad")

ad_select = section:taboption("advanced", ListValue, "ad_select",
	translate("Aggregation Selection Logic"),
	translate("Specifies the aggregation selection logic to use"))
ad_select.default = "stable"
ad_select:value("stable", translate("Aggregator: All slaves down or has no slaves (stable, 0)"))
ad_select:value("bandwidth", translate("Aggregator: Slave added/removed or state changes (bandwidth, 1)"))
ad_select:value("count", translate("Aggregator: Chosen by the largest number of ports + slave added/removed or state changes (count, 2)"))
ad_select:depends("bonding_policy", "802.3ad")

lacp_rate = section:taboption("advanced", ListValue, "lacp_rate",
	translate("LACPDU Packets"),
	translate("Specifies the rate in which the link partner will be asked to transmit LACPDU packets"))
lacp_rate.default = "slow"
lacp_rate:value("slow", translate("Every 30 seconds (slow, 0)"))
lacp_rate:value("fast", translate("Every second (fast, 1)"))
lacp_rate:depends("bonding_policy", "802.3ad")

packets_per_slave = section:taboption("advanced", Value, "packets_per_slave",
	translate("Packets To Transmit Before Moving To Next Slave"),
	translate("Specifies the number of packets to transmit through a slave before moving to the next one"))
packets_per_slave.datatype = "range(0,65535)"
packets_per_slave.default = 1
packets_per_slave:depends("bonding_policy", "balance-rr")

lp_interval = section:taboption("advanced", Value, "lp_interval",
	translate("Interval For Sending Learning Packets"),
	translate("Specifies the number of seconds between instances where the bonding	driver sends learning packets to each slaves peer switch"))
lp_interval.datatype = "range(1,2147483647)"
lp_interval.default = 1
lp_interval:depends("bonding_policy", "balance-tlb")
lp_interval:depends("bonding_policy", "balance-alb")

tlb_dynamic_lb = section:taboption("advanced", ListValue, "tlb_dynamic_lb",
	translate("Enable Dynamic Shuffling Of Flows"),
	translate("Specifies whether to shuffle active flows across slaves based on the load"))
tlb_dynamic_lb.default = "1"
tlb_dynamic_lb:value("1", translate("Yes"))
tlb_dynamic_lb:value("0", translate("No"))
tlb_dynamic_lb:depends("bonding_policy", "balance-tlb")

fail_over_mac = section:taboption("advanced", ListValue, "fail_over_mac",
	translate("Set same MAC Address to all slaves"),
	translate("Specifies whether active-backup mode should set all slaves to the same MAC address at enslavement"))
fail_over_mac.default = "none"
fail_over_mac:value("none", translate("Yes (none, 0)"))
fail_over_mac:value("active", translate("Set to currently active slave (active, 1)"))
fail_over_mac:value("follow", translate("Set to first slave added to the bond (follow, 2)"))
fail_over_mac:depends("bonding_policy", "active-backup")

num_grat_arp__num_unsol_na = section:taboption("advanced", Value, "num_grat_arp__num_unsol_na",
	translate("Number of peer notifications after failover event"),
	translate("Specifies the number of peer notifications (gratuitous ARPs and unsolicited IPv6 Neighbor Advertisements) to be issued after a failover event"))
num_grat_arp__num_unsol_na.datatype = "range(0,255)"
num_grat_arp__num_unsol_na.default = 1
num_grat_arp__num_unsol_na:depends("bonding_policy", "active-backup")

xmit_hash_policy = section:taboption("advanced", ListValue, "xmit_hash_policy",
	translate("Transmit Hash Policy"),
	translate("Selects the transmit hash policy to use for slave selection"))
xmit_hash_policy.default = "layer2"
xmit_hash_policy:value("layer2", translate("Use XOR of hardware MAC addresses (layer2)"))
xmit_hash_policy:value("layer2+3", translate("Use XOR of hardware MAC addresses and IP addresses (layer2+3)"))
xmit_hash_policy:value("layer3+4", translate("Use upper layer protocol information (layer3+4)"))
xmit_hash_policy:value("encap2+3", translate("Use XOR of hardware MAC addresses and IP addresses, rely on skb_flow_dissect (encap2+3)"))
xmit_hash_policy:value("encap3+4", translate("Use upper layer protocol information, rely on skb_flow_dissect (encap3+4)"))
xmit_hash_policy:depends("bonding_policy", "balance-rr")
xmit_hash_policy:depends("bonding_policy", "active-backup")
xmit_hash_policy:depends("bonding_policy", "balance-tlb")
xmit_hash_policy:depends("bonding_policy", "balance-alb")
xmit_hash_policy:depends("bonding_policy", "balance-xor")

resend_igmp = section:taboption("advanced", Value, "resend_igmp",
	translate("Number of IGMP membership reports"),
	translate("Specifies the number of IGMP membership reports to be issued after a failover event in 200ms intervals"))
resend_igmp.datatype = "range(0,255)"
resend_igmp.default = 1
resend_igmp:depends("bonding_policy", "balance-tlb")
resend_igmp:depends("bonding_policy", "balance-alb")

all_slaves_active = section:taboption("advanced", ListValue, "all_slaves_active",
	translate("Drop Duplicate Frames"),
	translate("Specifies that duplicate frames (received on inactive ports) should be dropped or delivered"))
all_slaves_active.default = "0"
all_slaves_active:value("0", translate("Yes"))
all_slaves_active:value("1", translate("No"))

link_monitoring = section:taboption("advanced", ListValue, "link_monitoring",
	translate("Link Monitoring"),
	translate("Method of link monitoring"))
link_monitoring.default = "off"
link_monitoring:value("off", translate("Off"))
link_monitoring:value("arp", translate("ARP"))
link_monitoring:value("mii", translate("MII"))

arp_interval = section:taboption("advanced", Value, "arp_interval",
	translate("ARP Interval"),
	translate("Specifies the ARP link monitoring frequency in milliseconds"))
arp_interval.datatype = "uinteger"
arp_interval.default = 0
arp_interval:depends("link_monitoring", "arp")

arp_ip_target = section:taboption("advanced", DynamicList, "arp_ip_target",
	translate("ARP IP Targets"),
	translate("Specifies the IP addresses to use for ARP monitoring"))
arp_ip_target.datatype = "ipaddr"
arp_ip_target.cast = "string"
arp_ip_target:depends("link_monitoring", "arp")

arp_all_targets = section:taboption("advanced", ListValue, "arp_all_targets",
	translate("ARP mode to consider a slave as being up"),
	translate("Specifies the quantity of ARP IP targets that must be reachable"))
arp_all_targets.default = "any"
arp_all_targets:value("any", translate("Consider the slave up when any ARP IP target is reachable (any, 0)"))
arp_all_targets:value("all", translate("Consider the slave up when all ARP IP targets are reachable (all, 1)"))
arp_all_targets:depends({link_monitoring="arp", bonding_policy="active-backup"})

arp_validate = section:taboption("advanced", ListValue, "arp_validate",
	translate("ARP Validation"),
	translate("Specifies whether ARP probes and replies should be validated or non-ARP traffic should be filtered for link monitoring"))
arp_validate.default = "filter"
arp_validate:value("none", translate("No validation or filtering "))
arp_validate:value("active", translate("Validation only for active slave"))
arp_validate:value("backup", translate("Validation only for backup slaves"))
arp_validate:value("all", translate("Validation for all slaves"))
arp_validate:value("filter", translate("Filtering for all slaves, no validation"))
arp_validate:value("filter_active", translate("Filtering for all slaves, validation only for active slave"))
arp_validate:value("filter_backup", translate("Filtering for all slaves, validation only for backup slaves"))
arp_validate:depends("link_monitoring", "arp")

miimon = section:taboption("advanced", Value, "miimon",
	translate("MII Interval"),
	translate("Specifies the MII link monitoring frequency in milliseconds"))
miimon.datatype = "uinteger"
miimon.default = 0
miimon:depends("link_monitoring", "mii")

downdelay = section:taboption("advanced", Value, "downdelay",
	translate("Down Delay"),
	translate("Specifies the time in milliseconds to wait before disabling a slave after a link failure detection"))
downdelay.datatype = "uinteger"
downdelay.default = 0
downdelay:depends("link_monitoring", "mii")

updelay = section:taboption("advanced", Value, "updelay",
	translate("Up Delay"),
	translate("Specifies the time in milliseconds to wait before enabling a slave after a link recovery detection"))
updelay.datatype = "uinteger"
updelay.default = 0
updelay:depends("link_monitoring", "mii")

use_carrier = section:taboption("advanced", ListValue, "use_carrier",
	translate("Method to determine link status"),
	translate("Specifies whether or not miimon should use MII or ETHTOOL ioctls vs. netif_carrier_ok()"))
use_carrier.default = "1"
use_carrier:value("0", translate("MII / ETHTOOL ioctls"))
use_carrier:value("1", translate("netif_carrier_ok()"))
use_carrier:depends("link_monitoring", "mii")


-- we use the bondig_policy validate function to check for other required values
-- (e.g. slave interfaces, primary interface, ARP targets, ...) as validate functions
-- are not called for _empty_ fields. bonding_policy will never be empty.
function bonding_policy.validate(self, value, section)

	local selected_link_monitoring = link_monitoring:formvalue(section)
	local selected_arp_ip_targets = arp_ip_target:formvalue(section)

	local selected_policy = bonding_policy:formvalue(section)

	local selected_slaves = slaves:formvalue(section)
	local selected_primary = primary:formvalue(section)

	if selected_link_monitoring == "arp" then
		if selected_policy == "802.3ad" or selected_policy == "balance-tlb" or selected_policy == "balance-alb" then
			return nil, translate("ARP monitoring is not supported for the selected policy")
		end

		if #selected_arp_ip_targets == 0 then
			return nil, translate("You must select at least one ARP IP target if ARP monitoring is selected")
		end

	end

	if selected_slaves == nil then
		return nil, translate("You must select at least one slave interface")
	end

	if selected_policy == "active-backup" or selected_policy == "balance-tlb" or selected_policy == "balance-alb" then
		if selected_primary == nil then
			return nil, translate("You must select a primary interface for the selected policy")
		else
			if (type(selected_slaves) == "table") then
				for key,slave_value in pairs(selected_slaves) do
					if slave_value == selected_primary then
						return value
					end
				end
			else
				if selected_slaves == selected_primary then
					return value
				end
			end
		end

		return nil, translate("You must select a primary interface which is included in selected slave interfaces")
	end

	return value
end

