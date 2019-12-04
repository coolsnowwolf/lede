-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local dsp = require "luci.dispatcher"
local uci = require "uci"

local m, mwan_rule, src_ip, src_port, dest_ip, dest_port, proto, use_policy

function ruleCheck()
	local rule_error = {}
	uci.cursor():foreach("mwan3", "rule",
		function (section)
			rule_error[section[".name"]] = false
			local uci = uci.cursor(nil, "/var/state")
			local sourcePort = uci:get("mwan3", section[".name"], "src_port")
			local destPort = uci:get("mwan3", section[".name"], "dest_port")
			if sourcePort ~= nil or destPort ~= nil then
				local protocol = uci:get("mwan3", section[".name"], "proto")
				if protocol == nil or protocol == "all" then
					rule_error[section[".name"]] = true
				end
			end
		end
	)
	return rule_error
end

function ruleWarn(rule_error)
	local warnings = ""
	for i, k in pairs(rule_error) do
		if rule_error[i] == true then
			warnings = warnings .. string.format("<strong>%s</strong><br />",
				translatef("WARNING: Rule %s have a port configured with no or improper protocol specified!", i)
				)
		end
	end

	return warnings
end

m = Map("mwan3", translate("MWAN - Rules"),
	ruleWarn(ruleCheck())
	)

mwan_rule = m:section(TypedSection, "rule", nil,
	translate("Rules specify which traffic will use a particular MWAN policy<br />" ..
	"Rules are based on IP address, port or protocol<br />" ..
	"Rules are matched from top to bottom<br />" ..
	"Rules below a matching rule are ignored<br />" ..
	"Traffic not matching any rule is routed using the main routing table<br />" ..
	"Traffic destined for known (other than default) networks is handled by the main routing table<br />" ..
	"Traffic matching a rule, but all WAN interfaces for that policy are down will be blackholed<br />" ..
	"Names may contain characters A-Z, a-z, 0-9, _ and no spaces<br />" ..
	"Rules may not share the same name as configured interfaces, members or policies"))
mwan_rule.addremove = true
mwan_rule.anonymous = false
mwan_rule.dynamic = false
mwan_rule.sectionhead = translate("Rule")
mwan_rule.sortable = true
mwan_rule.template = "cbi/tblsection"
mwan_rule.extedit = dsp.build_url("admin", "network", "mwan", "rule", "%s")
function mwan_rule.create(self, section)
	TypedSection.create(self, section)
	m.uci:save("mwan3")
	luci.http.redirect(dsp.build_url("admin", "network", "mwan", "rule", section))
end

src_ip = mwan_rule:option(DummyValue, "src_ip", translate("Source address"))
src_ip.rawhtml = true
function src_ip.cfgvalue(self, s)
	return self.map:get(s, "src_ip") or "&#8212;"
end

src_port = mwan_rule:option(DummyValue, "src_port", translate("Source port"))
src_port.rawhtml = true
function src_port.cfgvalue(self, s)
	return self.map:get(s, "src_port") or "&#8212;"
end

dest_ip = mwan_rule:option(DummyValue, "dest_ip", translate("Destination address"))
dest_ip.rawhtml = true
function dest_ip.cfgvalue(self, s)
	return self.map:get(s, "dest_ip") or "&#8212;"
end

dest_port = mwan_rule:option(DummyValue, "dest_port", translate("Destination port"))
dest_port.rawhtml = true
function dest_port.cfgvalue(self, s)
	return self.map:get(s, "dest_port") or "&#8212;"
end

proto = mwan_rule:option(DummyValue, "proto", translate("Protocol"))
proto.rawhtml = true
function proto.cfgvalue(self, s)
	return self.map:get(s, "proto") or "all"
end

use_policy = mwan_rule:option(DummyValue, "use_policy", translate("Policy assigned"))
use_policy.rawhtml = true
function use_policy.cfgvalue(self, s)
	return self.map:get(s, "use_policy") or "&#8212;"
end

return m
