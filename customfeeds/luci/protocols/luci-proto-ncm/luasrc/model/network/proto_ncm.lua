--[[
LuCI - Network model - NCM protocol extension

Copyright 2015 Cezary Jackiewicz <cezary.jackiewicz@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

]]--

local netmod = luci.model.network

local proto = netmod:register_protocol("ncm")
local interface = luci.model.network.interface

function proto.get_i18n(self)
	return luci.i18n.translate("NCM")
end

function proto.opkg_package(self)
	return "comgt-ncm"
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/ncm.sh")
end

function proto.is_floating(self)
	return true
end

function proto.is_virtual(self)
	return true
end

function proto.get_interface(self)
	local _ifname=netmod.protocol.ifname(self)
	if not _ifname then
		_ifname = "wan"
	end
	return interface(_ifname, self)
end

function proto.get_interfaces(self)
	return nil
end

function proto.contains_interface(self, ifc)
	return (netmod:ifnameof(ifc) == self:ifname())
end

netmod:register_pattern_virtual("^ncm%-%w")
