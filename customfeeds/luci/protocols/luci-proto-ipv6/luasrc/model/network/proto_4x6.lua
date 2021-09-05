-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2013 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local netmod = luci.model.network

local _, p
for _, p in ipairs({"dslite", "map", "464xlat"}) do

	local proto = netmod:register_protocol(p)

	function proto.get_i18n(self)
		if p == "dslite" then
			return luci.i18n.translate("Dual-Stack Lite (RFC6333)")
		elseif p == "map" then
			return luci.i18n.translate("MAP / LW4over6")
		elseif p == "464xlat" then
			return luci.i18n.translate("464XLAT (CLAT)")
		end
	end

	function proto.ifname(self)
		return p .. "-" .. self.sid
	end

	function proto.opkg_package(self)
		if p == "dslite" then
			return "ds-lite"
		elseif p == "map" then
			return "map-t"
		elseif p == "464xlat" then
			return "464xlat"
		end
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

	if p == "dslite" then
		netmod:register_pattern_virtual("^ds%-%w")
	elseif p == "map" then
		netmod:register_pattern_virtual("^map%-%w")
	elseif p == "464xlat" then
		netmod:register_pattern_virtual("^464%-%w")
	end
end
