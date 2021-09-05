-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2010-2012 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ds = require "luci.dispatcher"
local ft = require "luci.tools.firewall"

m = Map("firewall", translate("Firewall - Port Forwards"),
	translate("Port forwarding allows remote computers on the Internet to \
	           connect to a specific computer or service within the \
	           private LAN."))

--
-- Port Forwards
--

s = m:section(TypedSection, "redirect", translate("Port Forwards"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/network/firewall/forwards/%s")
s.template_addremove = "firewall/cbi_addforward"

function s.create(self, section)
	local n = m:formvalue("_newfwd.name")
	local p = m:formvalue("_newfwd.proto")
	local E = m:formvalue("_newfwd.extzone")
	local e = m:formvalue("_newfwd.extport")
	local I = m:formvalue("_newfwd.intzone")
	local a = m:formvalue("_newfwd.intaddr")
	local i = m:formvalue("_newfwd.intport")

	if p == "other" or (p and a) then
		created = TypedSection.create(self, section)

		self.map:set(created, "target",    "DNAT")
		self.map:set(created, "src",       E or "wan")
		self.map:set(created, "dest",      I or "lan")
		self.map:set(created, "proto",     (p ~= "other") and p or "all")
		self.map:set(created, "src_dport", e)
		self.map:set(created, "dest_ip",   a)
		self.map:set(created, "dest_port", i)
		self.map:set(created, "name",      n)
	end

	if p ~= "other" then
		created = nil
	end
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save("firewall")
		luci.http.redirect(ds.build_url(
			"admin/network/firewall/redirect", created
		))
	end
end

function s.filter(self, sid)
	return (self.map:get(sid, "target") ~= "SNAT")
end


ft.opt_name(s, DummyValue, translate("Name"))


local function forward_proto_txt(self, s)
	return "%s-%s" %{
		translate("IPv4"),
		ft.fmt_proto(self.map:get(s, "proto"),
	                 self.map:get(s, "icmp_type")) or "TCP+UDP"
	}
end

local function forward_src_txt(self, s)
	local z = ft.fmt_zone(self.map:get(s, "src"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "src_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "src_port") or "")
	local m = ft.fmt_mac(self.map:get(s, "src_mac") or "")

	if p and m then
		return translatef("From %s in %s with source %s and %s", a, z, p, m)
	elseif p or m then
		return translatef("From %s in %s with source %s", a, z, p or m)
	else
		return translatef("From %s in %s", a, z)
	end
end

local function forward_via_txt(self, s)
	local a = ft.fmt_ip(self.map:get(s, "src_dip"), translate("any router IP"))
	local p = ft.fmt_port(self.map:get(s, "src_dport") or "")

	if p then
		return translatef("Via %s at %s", a, p)
	else
		return translatef("Via %s", a)
	end
end

match = s:option(DummyValue, "match", translate("Match"))
match.rawhtml = true
match.width   = "50%"
function match.cfgvalue(self, s)
	return "<small>%s<br />%s<br />%s</small>" % {
		forward_proto_txt(self, s),
		forward_src_txt(self, s),
		forward_via_txt(self, s)
	}
end


dest = s:option(DummyValue, "dest", translate("Forward to"))
dest.rawhtml = true
dest.width   = "40%"
function dest.cfgvalue(self, s)
	local z = ft.fmt_zone(self.map:get(s, "dest"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "dest_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "dest_port") or "") or
		ft.fmt_port(self.map:get(s, "src_dport") or "")

	if p then
		return translatef("%s, %s in %s", a, p, z)
	else
		return translatef("%s in %s", a, z)
	end
end

ft.opt_enabled(s, Flag, translate("Enable")).width = "1%"

return m
