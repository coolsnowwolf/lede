-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local dsp = require "luci.dispatcher"
local uci = require "uci"

local m, mwan_policy, use_member, last_resort

function policyCheck()
	local policy_error = {}

	uci.cursor():foreach("mwan3", "policy",
		function (section)
			policy_error[section[".name"]] = false
			if string.len(section[".name"]) > 15 then
				policy_error[section[".name"]] = true
			end
		end
	)

	return policy_error
end

function policyError(policy_error)
	local warnings = ""
	for i, k in pairs(policy_error) do
		if policy_error[i] == true then
			warnings = warnings .. string.format("<strong>%s</strong><br />",
				translatef("WARNING: Policy %s has exceeding the maximum name of 15 characters", i)
				)
		end
	end

	return warnings
end

m = Map("mwan3", translate("MWAN - Policies"),
	policyError(policyCheck()))

mwan_policy = m:section(TypedSection, "policy", nil,
	translate("Policies are profiles grouping one or more members controlling how MWAN distributes traffic<br />" ..
	"Member interfaces with lower metrics are used first<br />" ..
	"Member interfaces with the same metric will be load-balanced<br />" ..
	"Load-balanced member interfaces distribute more traffic out those with higher weights<br />" ..
	"Names may contain characters A-Z, a-z, 0-9, _ and no spaces<br />" ..
	"Names must be 15 characters or less<br />" ..
	"Policies may not share the same name as configured interfaces, members or rules"))
mwan_policy.addremove = true
mwan_policy.dynamic = false
mwan_policy.sectionhead = translate("Policy")
mwan_policy.sortable = true
mwan_policy.template = "cbi/tblsection"
mwan_policy.extedit = dsp.build_url("admin", "network", "mwan", "policy", "%s")
function mwan_policy.create(self, section)
	TypedSection.create(self, section)
	m.uci:save("mwan3")
	luci.http.redirect(dsp.build_url("admin", "network", "mwan", "policy", section))
end

use_member = mwan_policy:option(DummyValue, "use_member", translate("Members assigned"))
use_member.rawhtml = true
function use_member.cfgvalue(self, s)
	local memberConfig, memberList = self.map:get(s, "use_member"), ""
	if memberConfig then
		for k,v in pairs(memberConfig) do
			memberList = memberList .. v .. "<br />"
		end
		return memberList
	else
		return "&#8212;"
	end
end

last_resort = mwan_policy:option(DummyValue, "last_resort", translate("Last resort"))
last_resort.rawhtml = true
function last_resort.cfgvalue(self, s)
	local action = self.map:get(s, "last_resort")
	if action == "blackhole" then
		return translate("blackhole (drop)")
	elseif action == "default" then
		return translate("default (use main routing table)")
	else
		return translate("unreachable (reject)")
	end
end

return m
