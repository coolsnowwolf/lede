-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2010-2012 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local ds = require "luci.dispatcher"
local ft = require "luci.tools.firewall"

m = Map("firewall",
	translate("Firewall - Traffic Rules"),
	translate("Traffic rules define policies for packets traveling between \
		different zones, for example to reject traffic between certain hosts \
		or to open WAN ports on the router."))

--
-- Rules
--

s = m:section(TypedSection, "rule", translate("Traffic Rules"))
s.addremove = true
s.anonymous = true
s.sortable  = true
s.template = "cbi/tblsection"
s.extedit   = ds.build_url("admin/network/firewall/rules/%s")
s.defaults.target = "ACCEPT"
s.template_addremove = "firewall/cbi_addrule"


function s.create(self, section)
	created = TypedSection.create(self, section)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	local i_n = m:formvalue("_newopen.name")
	local i_p = m:formvalue("_newopen.proto")
	local i_e = m:formvalue("_newopen.extport")
	local i_x = m:formvalue("_newopen.submit")

	local f_n = m:formvalue("_newfwd.name")
	local f_s = m:formvalue("_newfwd.src")
	local f_d = m:formvalue("_newfwd.dest")
	local f_x = m:formvalue("_newfwd.submit")

	if i_x then
		created = TypedSection.create(self, section)

		self.map:set(created, "target",    "ACCEPT")
		self.map:set(created, "src",       "wan")
		self.map:set(created, "proto",     (i_p ~= "other") and i_p or "all")
		self.map:set(created, "dest_port", i_e)
		self.map:set(created, "name",      i_n)

		if i_p ~= "other" and i_e and #i_e > 0 then
			created = nil
		end

	elseif f_x then
		created = TypedSection.create(self, section)

		self.map:set(created, "target", "ACCEPT")
		self.map:set(created, "src",    f_s)
		self.map:set(created, "dest",   f_d)
		self.map:set(created, "name",   f_n)
	end

	if created then
		m.uci:save("firewall")
		luci.http.redirect(ds.build_url(
			"admin/network/firewall/rules", created
		))
	end
end

ft.opt_name(s, DummyValue, translate("Name"))

local function rule_proto_txt(self, s)
	local f = self.map:get(s, "family")
	local p = ft.fmt_proto(self.map:get(s, "proto"),
	                       self.map:get(s, "icmp_type")) or translate("traffic")

	if f and f:match("4") then
		return "%s-%s" %{ translate("IPv4"), p }
	elseif f and f:match("6") then
		return "%s-%s" %{ translate("IPv6"), p }
	else
		return "%s %s" %{ translate("Any"), p }
	end
end

local function rule_src_txt(self, s)
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

local function rule_dest_txt(self, s)
	local z = ft.fmt_zone(self.map:get(s, "dest") or "")
	local p = ft.fmt_port(self.map:get(s, "dest_port") or "")

	-- Forward
	if z then
		local a = ft.fmt_ip(self.map:get(s, "dest_ip"), translate("any host"))
		if p then
			return translatef("To %s, %s in %s", a, p, z)
		else
			return translatef("To %s in %s", a, z)
		end

	-- Input
	else
		local a = ft.fmt_ip(self.map:get(s, "dest_ip"),
			translate("any router IP"))

		if p then
			return translatef("To %s at %s on <var>this device</var>", a, p)
		else
			return translatef("To %s on <var>this device</var>", a)
		end
	end
end

local function snat_dest_txt(self, s)
	local z = ft.fmt_zone(self.map:get(s, "dest"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "dest_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "dest_port")) or
		ft.fmt_port(self.map:get(s, "src_dport"))

	if p then
		return translatef("To %s, %s in %s", a, p, z)
	else
		return translatef("To %s in %s", a, z)
	end
end


match = s:option(DummyValue, "match", translate("Match"))
match.rawhtml = true
match.width   = "70%"
function match.cfgvalue(self, s)
	return "<small>%s<br />%s<br />%s</small>" % {
		rule_proto_txt(self, s),
		rule_src_txt(self, s),
		rule_dest_txt(self, s)
	}
end

target = s:option(DummyValue, "target", translate("Action"))
target.rawhtml = true
target.width   = "20%"
function target.cfgvalue(self, s)
	local t = ft.fmt_target(self.map:get(s, "target"), self.map:get(s, "dest"))
	local l = ft.fmt_limit(self.map:get(s, "limit"),
		self.map:get(s, "limit_burst"))

	if l then
		return translatef("<var>%s</var> and limit to %s", t, l)
	else
		return "<var>%s</var>" % t
	end
end

ft.opt_enabled(s, Flag, translate("Enable")).width = "1%"


--
-- SNAT
--

s = m:section(TypedSection, "redirect",
	translate("Source NAT"),
	translate("Source NAT is a specific form of masquerading which allows \
		fine grained control over the source IP used for outgoing traffic, \
		for example to map multiple WAN addresses to internal subnets."))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/network/firewall/rules/%s")
s.template_addremove = "firewall/cbi_addsnat"

function s.create(self, section)
	created = TypedSection.create(self, section)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	local n = m:formvalue("_newsnat.name")
	local s = m:formvalue("_newsnat.src")
	local d = m:formvalue("_newsnat.dest")
	local a = m:formvalue("_newsnat.dip")
	local p = m:formvalue("_newsnat.dport")
	local x = m:formvalue("_newsnat.submit")

	if x and a and #a > 0 then
		created = TypedSection.create(self, section)

		self.map:set(created, "target",    "SNAT")
		self.map:set(created, "src",       s)
		self.map:set(created, "dest",      d)
		self.map:set(created, "proto",     "all")
		self.map:set(created, "src_dip",   a)
		self.map:set(created, "src_dport", p)
		self.map:set(created, "name",      n)
	end

	if created then
		m.uci:save("firewall")
		luci.http.redirect(ds.build_url(
			"admin/network/firewall/rules", created
		))
	end
end

function s.filter(self, sid)
	return (self.map:get(sid, "target") == "SNAT")
end

ft.opt_name(s, DummyValue, translate("Name"))

match = s:option(DummyValue, "match", translate("Match"))
match.rawhtml = true
match.width   = "70%"
function match.cfgvalue(self, s)
	return "<small>%s<br />%s<br />%s</small>" % {
		rule_proto_txt(self, s),
		rule_src_txt(self, s),
		snat_dest_txt(self, s)
	}
end

snat = s:option(DummyValue, "via", translate("Action"))
snat.rawhtml = true
snat.width   = "20%"
function snat.cfgvalue(self, s)
	local a = ft.fmt_ip(self.map:get(s, "src_dip"))
	local p = ft.fmt_port(self.map:get(s, "src_dport"))

	if a and p then
		return translatef("Rewrite to source %s, %s", a, p)
	else
		return translatef("Rewrite to source %s", a or p)
	end
end

ft.opt_enabled(s, Flag, translate("Enable")).width = "1%"


return m
