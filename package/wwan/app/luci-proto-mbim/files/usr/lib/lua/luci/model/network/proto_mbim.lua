-- Copyright 2016 David Thornley <david.thornley@touchstargroup.com>
-- Licensed to the public under the Apache License 2.0.

local netmod = luci.model.network
local interface = luci.model.network.interface
local proto = netmod:register_protocol("mbim")

function proto.get_i18n(self)
	return luci.i18n.translate("MBIM Cellular")
end

function proto.ifname(self)
	local base = netmod._M.protocol
	local ifname = base.ifname(self) -- call base class "protocol.ifname(self)"

		-- Note: ifname might be nil if the adapter could not be determined through ubus (default name to mbim-wan in this case)
	if ifname == nil then
		ifname = "mbim-" .. self.sid
	end
	return ifname
end

function proto.get_interface(self)
	return interface(self:ifname(), self)
end

function proto.opkg_package(self)
	return "rmbim"
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/mbim.sh")
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

netmod:register_pattern_virtual("^mbim%-%w")

netmod:register_error_code("CALL_FAILED",	luci.i18n.translate("Call failed"))
netmod:register_error_code("NO_CID",		luci.i18n.translate("Unable to obtain client ID"))
netmod:register_error_code("PLMN_FAILED",	luci.i18n.translate("Setting PLMN failed"))
