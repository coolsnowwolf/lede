-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local wa = require "luci.tools.webadmin"
local fs = require "nixio.fs"

m = Map("qos", translate("Quality of Service"),
	translate("With <abbr title=\"Quality of Service\">QoS</abbr> you " ..
		"can prioritize network traffic selected by addresses, " ..
		"ports or services."))

s = m:section(TypedSection, "interface", translate("Interfaces"))
s.addremove = true
s.anonymous = false

e = s:option(Flag, "enabled", translate("Enable"))
e.rmempty = false

c = s:option(ListValue, "classgroup", translate("Classification group"))
c:value("Default", translate("default"))
c.default = "Default"

s:option(Flag, "overhead", translate("Calculate overhead"))

s:option(Flag, "halfduplex", translate("Half-duplex"))

dl = s:option(Value, "download", translate("Download speed (kbit/s)"))
dl.datatype = "and(uinteger,min(1))"

ul = s:option(Value, "upload", translate("Upload speed (kbit/s)"))
ul.datatype = "and(uinteger,min(1))"

s = m:section(TypedSection, "classify", translate("Classification Rules"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true
s.sortable  = true

t = s:option(ListValue, "target", translate("Target"))
t:value("Priority", translate("priority"))
t:value("Express", translate("express"))
t:value("Normal", translate("normal"))
t:value("Bulk", translate("low"))

local uci = require "luci.model.uci"
uci.cursor():foreach("qos", "class",
    function (section)
        local n = section[".name"]
        if string.sub(n,-string.len("_down"))~="_down" then
            t:value(n)
        end
    end)

t.default = "Normal"

srch = s:option(Value, "srchost", translate("Source host"))
srch.rmempty = true
srch:value("", translate("all"))
wa.cbi_add_knownips(srch)

dsth = s:option(Value, "dsthost", translate("Destination host"))
dsth.rmempty = true
dsth:value("", translate("all"))
wa.cbi_add_knownips(dsth)

p = s:option(Value, "proto", translate("Protocol"))
p:value("", translate("all"))
p:value("tcp", "TCP")
p:value("udp", "UDP")
p:value("icmp", "ICMP")
p.rmempty = true

ports = s:option(Value, "ports", translate("Ports"))
ports.rmempty = true
ports:value("", translate("all"))

bytes = s:option(Value, "connbytes", translate("Number of bytes"))

comment = s:option(Value, "comment", translate("Comment"))

return m
