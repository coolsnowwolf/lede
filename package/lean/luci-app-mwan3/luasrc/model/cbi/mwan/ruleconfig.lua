-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local dsp = require "luci.dispatcher"
local util   = require("luci.util")

local m, mwan_rule, src_ip, src_port, dest_ip, dest_port, proto, sticky
local timeout, ipset, logging, policy

arg[1] = arg[1] or ""

local ipsets = util.split(util.trim(util.exec("ipset -n -L 2>/dev/null | grep -v mwan3_ | sort")), "\n", nil, true) or {}

m = Map("mwan3", translatef("MWAN Rule Configuration - %s", arg[1]))
m.redirect = dsp.build_url("admin", "network", "mwan", "rule")

mwan_rule = m:section(NamedSection, arg[1], "rule", "")
mwan_rule.addremove = false
mwan_rule.dynamic = false

src_ip = mwan_rule:option(Value, "src_ip", translate("Source address"),
	translate("Supports CIDR notation (eg \"192.168.100.0/24\") without quotes"))
src_ip.datatype = ipaddr

src_port = mwan_rule:option(Value, "src_port", translate("Source port"),
	translate("May be entered as a single or multiple port(s) (eg \"22\" or \"80,443\") or as a portrange (eg \"1024:2048\") without quotes"))

dest_ip = mwan_rule:option(Value, "dest_ip", translate("Destination address"),
	translate("Supports CIDR notation (eg \"192.168.100.0/24\") without quotes"))
dest_ip.datatype = ipaddr

dest_port = mwan_rule:option(Value, "dest_port", translate("Destination port"),
	translate("May be entered as a single or multiple port(s) (eg \"22\" or \"80,443\") or as a portrange (eg \"1024:2048\") without quotes"))

proto = mwan_rule:option(Value, "proto", translate("Protocol"),
	translate("View the content of /etc/protocols for protocol description"))
proto.default = "all"
proto.rmempty = false
proto:value("all")
proto:value("tcp")
proto:value("udp")
proto:value("icmp")
proto:value("esp")

sticky = mwan_rule:option(ListValue, "sticky", translate("Sticky"),
	translate("Traffic from the same source IP address that previously matched this rule within the sticky timeout period will use the same WAN interface"))
sticky.default = "0"
sticky:value("1", translate("Yes"))
sticky:value("0", translate("No"))

timeout = mwan_rule:option(Value, "timeout", translate("Sticky timeout"),
	translate("Seconds. Acceptable values: 1-1000000. Defaults to 600 if not set"))
timeout.datatype = "range(1, 1000000)"

ipset = mwan_rule:option(Value, "ipset", translate("IPset"),
	translate("Name of IPset rule. Requires IPset rule in /etc/dnsmasq.conf (eg \"ipset=/youtube.com/youtube\")"))
ipset:value("", translate("-- Please choose --"))
for _, z in ipairs(ipsets) do
	ipset:value(z)
end

logging = mwan_rule:option(Flag, "logging", translate("Logging"),
	translate("Enables firewall rule logging (global mwan3 logging must also be enabled)"))

policy = mwan_rule:option(Value, "use_policy", translate("Policy assigned"))
m.uci:foreach("mwan3", "policy",
	function(s)
		policy:value(s['.name'], s['.name'])
	end
)
policy:value("unreachable", translate("unreachable (reject)"))
policy:value("blackhole", translate("blackhole (drop)"))
policy:value("default", translate("default (use main routing table)"))

return m
