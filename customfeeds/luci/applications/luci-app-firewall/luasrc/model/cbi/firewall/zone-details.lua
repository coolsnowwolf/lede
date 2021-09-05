-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2010-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local ds = require "luci.dispatcher"
local ut = require "luci.util"

local m, p, i, v
local s, name, net, family, msrc, mdest, log, lim
local s2, out, inp


m = Map("firewall", translate("Firewall - Zone Settings"))
m.redirect = luci.dispatcher.build_url("admin/network/firewall/zones")

fw.init(m.uci)
nw.init(m.uci)


local zone = fw:get_zone(arg[1])
if not zone then
	luci.http.redirect(ds.build_url("admin/network/firewall/zones"))
	return
else
	m.title = "%s - %s" %{
		translate("Firewall - Zone Settings"),
		translatef("Zone %q", zone:name() or "?")
	}
end


s = m:section(NamedSection, zone.sid, "zone",
	translatef("Zone %q", zone:name()),
	translatef("This section defines common properties of %q. \
		The <em>input</em> and <em>output</em> options set the default \
		policies for traffic entering and leaving this zone while the \
		<em>forward</em> option describes the policy for forwarded traffic \
		between different networks within the zone. \
		<em>Covered networks</em> specifies which available networks are \
		members of this zone.", zone:name()))

s.anonymous = true
s.addremove = false

m.on_commit = function(map)
	local zone = fw:get_zone(arg[1])
	if zone then
		s.section  = zone.sid
		s2.section = zone.sid
	end
end


s:tab("general", translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))


name = s:taboption("general", Value, "name", translate("Name"))
name.optional = false
name.forcewrite = true
name.datatype = "and(uciname,maxlength(11))"

function name.write(self, section, value)
	if zone:name() ~= value then
		fw:rename_zone(zone:name(), value)
		out.exclude = value
		inp.exclude = value
	end
end

p = {
	s:taboption("general", ListValue, "input", translate("Input")),
	s:taboption("general", ListValue, "output", translate("Output")),
	s:taboption("general", ListValue, "forward", translate("Forward"))
}

for i, v in ipairs(p) do
	v:value("REJECT", translate("reject"))
	v:value("DROP", translate("drop"))
	v:value("ACCEPT", translate("accept"))
end

s:taboption("general", Flag, "masq", translate("Masquerading"))
s:taboption("general", Flag, "mtu_fix", translate("MSS clamping"))

net = s:taboption("general", Value, "network", translate("Covered networks"))
net.template = "cbi/network_netlist"
net.widget = "checkbox"
net.cast = "string"

function net.formvalue(self, section)
	return Value.formvalue(self, section) or "-"
end

function net.cfgvalue(self, section)
	return Value.cfgvalue(self, section) or name:cfgvalue(section)
end

function net.write(self, section, value)
	zone:clear_networks()

	local n
	for n in ut.imatch(value) do
		zone:add_network(n)
	end
end


family = s:taboption("advanced", ListValue, "family",
	translate("Restrict to address family"))

family.rmempty = true
family:value("", translate("IPv4 and IPv6"))
family:value("ipv4", translate("IPv4 only"))
family:value("ipv6", translate("IPv6 only"))

msrc = s:taboption("advanced", DynamicList, "masq_src",
	translate("Restrict Masquerading to given source subnets"))

msrc.optional = true
msrc.datatype = "list(neg(or(uciname,hostname,ipmask4)))"
msrc.placeholder = "0.0.0.0/0"
msrc:depends("family", "")
msrc:depends("family", "ipv4")

mdest = s:taboption("advanced", DynamicList, "masq_dest",
	translate("Restrict Masquerading to given destination subnets"))

mdest.optional = true
mdest.datatype = "list(neg(or(uciname,hostname,ipmask4)))"
mdest.placeholder = "0.0.0.0/0"
mdest:depends("family", "")
mdest:depends("family", "ipv4")

s:taboption("advanced", Flag, "conntrack",
	translate("Force connection tracking"))

log = s:taboption("advanced", Flag, "log",
	translate("Enable logging on this zone"))

log.rmempty = true
log.enabled = "1"

lim = s:taboption("advanced", Value, "log_limit",
	translate("Limit log messages"))

lim.placeholder = "10/minute"
lim:depends("log", "1")


s2 = m:section(NamedSection, zone.sid, "fwd_out",
	translate("Inter-Zone Forwarding"),
	translatef("The options below control the forwarding policies between \
		this zone (%s) and other zones. <em>Destination zones</em> cover \
		forwarded traffic <strong>originating from %q</strong>. \
		<em>Source zones</em> match forwarded traffic from other zones \
		<strong>targeted at %q</strong>. The forwarding rule is \
		<em>unidirectional</em>, e.g. a forward from lan to wan does \
		<em>not</em> imply a permission to forward from wan to lan as well.",
		zone:name(), zone:name(), zone:name()

	))

out = s2:option(Value, "out",
	translate("Allow forward to <em>destination zones</em>:"))

out.nocreate = true
out.widget = "checkbox"
out.exclude = zone:name()
out.template = "cbi/firewall_zonelist"

inp = s2:option(Value, "in",
	translate("Allow forward from <em>source zones</em>:"))

inp.nocreate = true
inp.widget = "checkbox"
inp.exclude = zone:name()
inp.template = "cbi/firewall_zonelist"

function out.cfgvalue(self, section)
	local v = { }
	local f
	for _, f in ipairs(zone:get_forwardings_by("src")) do
		v[#v+1] = f:dest()
	end
	return table.concat(v, " ")
end

function inp.cfgvalue(self, section)
	local v = { }
	local f
	for _, f in ipairs(zone:get_forwardings_by("dest")) do
		v[#v+1] = f:src()
	end
	return v
end

function out.formvalue(self, section)
	return Value.formvalue(self, section) or "-"
end

function inp.formvalue(self, section)
	return Value.formvalue(self, section) or "-"
end

function out.write(self, section, value)
	zone:del_forwardings_by("src")

	local f
	for f in ut.imatch(value) do
		zone:add_forwarding_to(f)
	end
end

function inp.write(self, section, value)
	zone:del_forwardings_by("dest")

	local f
	for f in ut.imatch(value) do
		zone:add_forwarding_from(f)
	end
end

return m
