--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2005-2013 hackpascal <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require "luci.util"
local uci = luci.model.uci.cursor_state()
local net = require "luci.model.network"

m = Map("usb_printer", translate("USB Printer Server"),
	translate("Shares multiple USB printers via TCP/IP.<br />When modified bingings, re-plug usb connectors to take effect.<br />This module requires kmod-usb-printer."))

function hex_align(hex, num)
	local len = num - string.len(hex)

	return string.rep("0", len) .. hex
end

function detect_usb_printers()
	local data = {}

	local lps = luci.util.execi("/usr/bin/detectlp")

	for value in lps do
		local row = {}

		--[[
			detectlp 的输出格式：
			设备名，VID/PID/?,描述，型号
		]]--

		local pos = string.find(value, ",")

		local devname = string.sub(value, 1, pos - 1)

		local value = string.sub(value, pos + 1, string.len(value))

		pos = string.find(value, ",")
		local product = string.sub(value, 1, pos - 1)

		value = string.sub(value, pos + 1, string.len(value))

		pos = string.find(value, ",")
		local model = string.sub(value, 1, pos - 1)

		local name = string.sub(value, pos + 1, string.len(value))

		pos = string.find(product, "/");

		local vid = string.sub(product, 1, pos - 1)

		local pid = string.sub(product, pos + 1, string.len(product))

		pos = string.find(pid, "/")
		pid = string.sub(pid, 1, pos - 1)

		row["description"] = name
		row["model"] = model
		row["id"] = hex_align(vid, 4) .. ":" .. hex_align(pid, 4)
		row["name"] = devname
		row["product"] = product

		table.insert(data, row)
	end

	return data
end

local printers = detect_usb_printers()

v = m:section(Table, printers, translate("Detected printers"))

v:option(DummyValue, "description", translate("Description"))
v:option(DummyValue, "model", translate("Printer Model"))
v:option(DummyValue, "id", translate("VID/PID"))
v:option(DummyValue, "name", translate("Device Name"))

net = net.init(m.uci)

s = m:section(TypedSection, "printer", translate("Bindings"))
s.addremove = true
s.anonymous = true

s:option(Flag, "enabled", translate("enable"))

d = s:option(Value, "device", translate("Device"))
d.rmempty = true

for key, item in ipairs(printers) do
	d:value(item["product"], item["description"] .. " [" .. item["id"] .. "]")
end

b = s:option(Value, "bind", translate("Interface"), translate("Specifies the interface to listen on."))
b.template = "cbi/network_netlist"
b.nocreate = true
b.unspecified = true

function b.cfgvalue(...)
	local v = Value.cfgvalue(...)
	if v then
		return (net:get_status_by_address(v))
	end
end

function b.write(self, section, value)
	local n = net:get_network(value)
	if n and n:ipaddr() then
		Value.write(self, section, n:ipaddr())
	end
end

p = s:option(ListValue, "port", translate("Port"), translate("TCP listener port."))
p.rmempty = true
for i = 0, 9 do
	p:value(i, 9100 + i)
end

s:option(Flag, "bidirectional", translate("Bidirectional mode"))

return m
