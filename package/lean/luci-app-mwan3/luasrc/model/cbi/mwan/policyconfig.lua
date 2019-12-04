-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local dsp = require "luci.dispatcher"

local m, mwan_policy, member, last_resort

arg[1] = arg[1] or ""

m = Map("mwan3", translatef("MWAN Policy Configuration - %s", arg[1]))
m.redirect = dsp.build_url("admin", "network", "mwan", "policy")

mwan_policy = m:section(NamedSection, arg[1], "policy", "")
mwan_policy.addremove = false
mwan_policy.dynamic = false

member = mwan_policy:option(DynamicList, "use_member", translate("Member used"))
m.uci:foreach("mwan3", "member",
	function(s)
		member:value(s['.name'], s['.name'])
	end
)

last_resort = mwan_policy:option(ListValue, "last_resort", translate("Last resort"),
	translate("When all policy members are offline use this behavior for matched traffic"))
last_resort.default = "unreachable"
last_resort:value("unreachable", translate("unreachable (reject)"))
last_resort:value("blackhole", translate("blackhole (drop)"))
last_resort:value("default", translate("default (use main routing table)"))

return m
