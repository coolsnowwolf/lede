-- Copyright 2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local wa   = require "luci.tools.webadmin"
local uci  = require "luci.model.uci".cursor()
local dsp  = require "luci.dispatcher"
local http = require "luci.http"
local qos  = require "luci.model.qos_gargoyle"

local m, class_s, rule_s, o
local upload_classes = {}
local qos_gargoyle = "qos_gargoyle"

uci:foreach(qos_gargoyle, "upload_class", function(s)
	local class_alias = s.name
	if class_alias then
		upload_classes[#upload_classes + 1] = {name = s[".name"], alias = class_alias}
	end
end)

m = Map(qos_gargoyle, translate("Upload Settings"))
m.template = "qos_gargoyle/list_view"

class_s = m:section(TypedSection, "upload_class", translate("Service Classes"),
	translate("Each upload service class is specified by three parameters: percent bandwidth at "
	.. "capacity, minimum bandwidth and maximum bandwidth."))
class_s.anonymous = true
class_s.addremove = true
class_s.template  = "cbi/tblsection"
class_s.extedit   = dsp.build_url("admin/network/qos_gargoyle/upload/class/%s")
class_s.create    = function(...)
	local sid = TypedSection.create(...)
	if sid then
		m.uci:save(qos_gargoyle)
		http.redirect(class_s.extedit % sid)
		return
	end
end

o = class_s:option(DummyValue, "name", translate("Class Name"))
o.cfgvalue = function(...)
	return Value.cfgvalue(...) or translate("None")
end

o = class_s:option(DummyValue, "percent_bandwidth", translate("Percent Bandwidth At Capacity"))
o.cfgvalue = function(...)
	local v = tonumber(Value.cfgvalue(...))
	if v and v > 0 then
		return "%d %%" % v
	end
	return translate("Not set")
end

o = class_s:option(DummyValue, "min_bandwidth", "%s (kbps)" % translate("Minimum Bandwidth"))
o.cfgvalue = function(...)
	local v = tonumber(Value.cfgvalue(...))
	return v or translate("Zero")
end

o = class_s:option(DummyValue, "max_bandwidth", "%s (kbps)" % translate("Maximum Bandwidth"))
o.cfgvalue = function(...)
	local v = tonumber(Value.cfgvalue(...))
	return v or translate("Unlimited")
end

o = class_s:option(DummyValue, "_ld", "%s (kbps)" % translate("Load"))
o.rawhtml = true
o.value   = "<em class=\"ld-upload\">*</em>"

rule_s = m:section(TypedSection, "upload_rule",translate("Classification Rules"),
	translate("Packets are tested against the rules in the order specified -- rules toward the top "
	.. "have priority. As soon as a packet matches a rule it is classified, and the rest of the rules "
	.. "are ignored. The order of the rules can be altered using the arrow controls.")
)
rule_s.addremove = true
rule_s.sortable  = true
rule_s.anonymous = true
rule_s.template  = "cbi/tblsection"
rule_s.extedit   = dsp.build_url("admin/network/qos_gargoyle/upload/rule/%s")
rule_s.create    = function(...)
	local sid = TypedSection.create(...)
	if sid then
		m.uci:save(qos_gargoyle)
		http.redirect(rule_s.extedit % sid)
		return
	end
end

o = rule_s:option(ListValue, "class", translate("Service Class"))
for _, s in ipairs(upload_classes) do o:value(s.name, s.alias) end

o = rule_s:option(Value, "proto", translate("Transport Protocol"))
o:value("", translate("All"))
o:value("tcp", "TCP")
o:value("udp", "UDP")
o:value("icmp", "ICMP")
o:value("gre", "GRE")
o.size = "10"
o.cfgvalue = function(...)
	local v = Value.cfgvalue(...)
	return v and v:upper() or ""
end
o.write = function(self, section, value)
	Value.write(self, section, value:lower())
end

o = rule_s:option(Value, "source", translate("Source IP(s)"))
o:value("", translate("All"))
wa.cbi_add_knownips(o)
o.datatype = "ipmask4"

o = rule_s:option(Value, "srcport", translate("Source Port(s)"))
o:value("", translate("All"))
o.datatype = "or(port, portrange)"

o = rule_s:option(Value, "destination", translate("Destination IP(s)"))
o:value("", translate("All"))
wa.cbi_add_knownips(o)
o.datatype = "ipmask4"

o = rule_s:option(Value, "dstport", translate("Destination Port(s)"))
o:value("", translate("All"))
o.datatype = "or(port, portrange)"

o = rule_s:option(DummyValue, "min_pkt_size", translate("Minimum Packet Length"))
o.cfgvalue = function(...)
	local v = tonumber(Value.cfgvalue(...))
	if v and v > 0 then
		return wa.byte_format(v)
	end
	return translate("Not set")
end

o = rule_s:option(DummyValue, "max_pkt_size", translate("Maximum Packet Length"))
o.cfgvalue = function(...)
	local v = tonumber(Value.cfgvalue(...))
	if v and v > 0 then
		return wa.byte_format(v)
	end
	return translate("Not set")
end

o = rule_s:option(DummyValue, "connbytes_kb", translate("Connection Bytes Reach"))
o.cfgvalue = function(...)
	local v = tonumber(Value.cfgvalue(...))
	if v and v > 0 then
		return wa.byte_format(v * 1024)
	end
	return translate("Not set")
end

if qos.has_ndpi() then
	o = rule_s:option(DummyValue, "ndpi", translate("DPI Protocol"))
	o.cfgvalue = function(...)
		local v = Value.cfgvalue(...)
		return v and v:upper() or translate("All")
	end
end

return m
