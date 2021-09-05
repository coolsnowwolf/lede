-- Copyright 2010-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local utl = require "luci.util"
local sys = require "luci.sys"
local fs  = require "nixio.fs"
local nw  = require "luci.model.network"

local dbdir, line

for line in io.lines("/etc/vnstat.conf") do
	dbdir = line:match("^%s*DatabaseDir%s+[\"'](%S-)[\"']")
	if dbdir then break end
end

dbdir = dbdir or "/var/lib/vnstat"


m = Map("vnstat", translate("VnStat"),
	translate("VnStat is a network traffic monitor for Linux that keeps a log of network traffic for the selected interface(s)."))

m.submit = translate("Restart VnStat")
m.reset  = false

nw.init(luci.model.uci.cursor_state())

local ifaces = { }
local enabled = { }
local iface

if fs.access(dbdir) then
	for iface in fs.dir(dbdir) do
		if iface:sub(1,1) ~= '.' then
			ifaces[iface] = iface
			enabled[iface] = iface
		end
	end
end

for _, iface in ipairs(sys.net.devices()) do
	ifaces[iface] = iface
end


local s = m:section(TypedSection, "vnstat")
s.anonymous = true
s.addremove = false

mon_ifaces = s:option(Value, "interface", translate("Monitor selected interfaces"))
mon_ifaces.template   = "cbi/network_ifacelist"
mon_ifaces.widget     = "checkbox"
mon_ifaces.cast       = "table"
mon_ifaces.noinactive = true
mon_ifaces.nocreate   = true

function mon_ifaces.write(self, section, val)
	local i
	local s = { }

	if val then
		for _, i in ipairs(type(val) == "table" and val or { val }) do
			s[i] = true
		end
	end

	for i, _ in pairs(ifaces) do
		if not s[i] then
			fs.unlink(dbdir .. "/" .. i)
			fs.unlink(dbdir .. "/." .. i)
		end
	end

	if next(s) then
		m.uci:set_list("vnstat", section, "interface", utl.keys(s))
	else
		m.uci:delete("vnstat", section, "interface")
	end
end

mon_ifaces.remove = mon_ifaces.write

return m
