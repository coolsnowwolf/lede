-- Copyright 2016 Roger Pueyo Centelles <roger.pueyo@guifi.net>
-- Licensed to the public under the Apache License 2.0.

local netmod = luci.model.network
local interface = luci.model.network.interface
local proto = netmod:register_protocol("ipip")

function proto.get_i18n(self)
	return luci.i18n.translate("IPv4-in-IPv4 (RFC2003)")
end

function proto.ifname(self)
	return "ipip-" .. self.sid
end

function proto.opkg_package(self)
	return "ipip"
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/ipip.sh")
end

function proto.is_floating(self)
	return true
end

function proto.is_virtual(self)
	return true
end

function proto.get_interfaces(self)
	return nil
end

function proto.contains_interface(self, ifc)
	 return (netmod:ifnameof(ifc) == self:ifname())
end

netmod:register_pattern_virtual("^ipip%-%w")
