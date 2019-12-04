-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local dsp = require "luci.dispatcher"

local m, mwan_member, interface, metric, weight

m = Map("mwan3", translate("MWAN - Members"))

mwan_member = m:section(TypedSection, "member", nil,
	translate("Members are profiles attaching a metric and weight to an MWAN interface<br />" ..
	"Names may contain characters A-Z, a-z, 0-9, _ and no spaces<br />" ..
	"Members may not share the same name as configured interfaces, policies or rules"))
mwan_member.addremove = true
mwan_member.dynamic = false
mwan_member.sectionhead = translate("Member")
mwan_member.sortable = true
mwan_member.template = "cbi/tblsection"
mwan_member.extedit = dsp.build_url("admin", "network", "mwan", "member", "%s")
function mwan_member.create(self, section)
	TypedSection.create(self, section)
	m.uci:save("mwan3")
	luci.http.redirect(dsp.build_url("admin", "network", "mwan", "member", section))
end

interface = mwan_member:option(DummyValue, "interface", translate("Interface"))
interface.rawhtml = true
function interface.cfgvalue(self, s)
	return self.map:get(s, "interface") or "&#8212;"
end

metric = mwan_member:option(DummyValue, "metric", translate("Metric"))
metric.rawhtml = true
function metric.cfgvalue(self, s)
	return self.map:get(s, "metric") or "1"
end

weight = mwan_member:option(DummyValue, "weight", translate("Weight"))
weight.rawhtml = true
function weight.cfgvalue(self, s)
	return self.map:get(s, "weight") or "1"
end

return m
