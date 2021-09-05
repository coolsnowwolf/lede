-- Copyright 2015 Paul Oranje <por@xs4all.nl>
-- Licensed to the public under GPLv2

local netmod = luci.model.network
local interface = luci.model.network.interface

local proto = netmod:register_protocol("aiccu")

function proto.get_i18n(self)
	return luci.i18n.translate("AICCU (SIXXS)")
end

function proto.ifname(self)
	return "aiccu-" .. self.sid
end

function proto.get_interface(self)
	return interface(self:ifname(), self)
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/aiccu.sh")
end

function proto.opkg_package(self)
	return "aiccu"
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

function proto.contains_interface(self, ifname)
	if self:is_floating() then
		return (netmod:ifnameof(ifc) == self:ifname())
	else
		return netmod.protocol.contains_interface(self, ifc)
	end
end

netmod:register_pattern_virtual("^aiccu%-%w")
