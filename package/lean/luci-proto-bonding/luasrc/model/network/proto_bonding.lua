--[[

Copyright (C) 2018 TDT AG <development@tdt.de>

This is free software, licensed under the Apache License Version 2.0.
See https://www.apache.org/licenses/LICENSE-2.0 for more information.

]]--

local proto = luci.model.network:register_protocol("bonding")

function proto.get_i18n(self)
	return luci.i18n.translate("Link Aggregation (Channel Bonding)")
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/bonding.sh")
end

function proto.is_virtual(self)
	return true
end

function proto.is_floating(self)
	return true
end

function proto.opkg_package(self)
	return "bonding"
end

