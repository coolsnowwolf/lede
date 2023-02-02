-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the Apache License 2.0.

local netmod = luci.model.network
local interface = luci.model.network.interface

local proto = netmod:register_protocol("3x")

function proto.get_i18n(self)
	return luci.i18n.translate("UMTS/GPRS/EV-DO")
end

function proto.ifname(self)
	return "3x-" .. self.sid
end

function proto.get_interface(self)
	return interface(self:ifname(), self)
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/3x.sh")
end

function proto.opkg_package(self)
	return "comgt"
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
	if self:is_floating() then
		return (netmod:ifnameof(ifc) == self:ifname())
	else
		return netmod.protocol.contains_interface(self, ifc)
	end
end

netmod:register_pattern_virtual("^3x%-%w")
