-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local netmod = luci.model.network

local _, p
for _, p in ipairs({"6in4", "6to4", "6rd"}) do

	local proto = netmod:register_protocol(p)

	function proto.get_i18n(self)
		if p == "6in4" then
			return luci.i18n.translate("IPv6-in-IPv4 (RFC4213)")
		elseif p == "6to4" then
			return luci.i18n.translate("IPv6-over-IPv4 (6to4)")
		elseif p == "6rd" then
			return luci.i18n.translate("IPv6-over-IPv4 (6rd)")
		end
	end

	function proto.ifname(self)
		return p .. "-" .. self.sid
	end

	function proto.opkg_package(self)
		return p
	end

	function proto.is_installed(self)
		return nixio.fs.access("/lib/netifd/proto/" .. p .. ".sh")
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
		return (netmod:ifnameof(ifc) == self:ifname())
	end

	netmod:register_pattern_virtual("^%s%%-%%w" % p)
end
