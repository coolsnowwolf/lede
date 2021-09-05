-- Copyright 2017 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local net = require "luci.model.network".init()


m = Map("mwan3", translate("MWAN - Globals"))

s = m:section(NamedSection, "globals", "globals", nil)
n = s:option(ListValue, "local_source",
	translate("Local source interface"),
	translate("Use the IP address of this interface as source IP " ..
	"address for traffic initiated by the router itself"))
n:value("none")
n.default = "none"
for _, net in ipairs(net:get_networks()) do
	if net:name() ~= "loopback" then
		n:value(net:name())
	end
end
n.rmempty = false

mask = s:option(
	Value,
	"mmx_mask",
	translate("Firewall mask"),
	translate("Enter value in hex, starting with <code>0x</code>"))
mask.datatype = "hex(4)"
mask.default = "0xff00"

return m
