-- Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

require "luci.sys"

local m, s, o


m = Map("luci_statistics",
	translate("OpenVPN Plugin Configuration"),
	translate("The OpenVPN plugin gathers information about the current vpn connection status."))

s = m:section( NamedSection, "collectd_openvpn", "luci_statistics" )


o = s:option( Flag, "enable", translate("Enable this plugin") )
o.default = "0"


o = s:option(Flag, "CollectIndividualUsers", translate("Generate a separate graph for each logged user"))
o.default = "0"
o.rmempty = true
o:depends("enable", 1)


o = s:option(Flag, "CollectUserCount", translate("Aggregate number of connected users"))
o.default = "0"
o.rmempty = true
o:depends("enable", 1)


o = s:option(Flag, "CollectCompression", translate("Gather compression statistics"))
o.default = "0"
o.rmempty = true
o:depends("enable", 1)


o = s:option(Flag, "ImprovedNamingSchema", translate("Use improved naming schema"))
o.default = "0"
o.rmempty = true
o:depends("enable", 1)


o = s:option(DynamicList, "StatusFile", translate("OpenVPN status files"))
o.rmempty = true
o:depends("enable", 1)

local status_files = nixio.fs.glob("/var/run/openvpn.*.status")
if status_files then
	local status_file
	for status_file in status_files do
		o:value(status_file)
	end
end

return m
