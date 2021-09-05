-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local netmod = luci.model.network

local _, p
for _, p in ipairs({"ppp", "pptp", "pppoe", "pppoa", "3g", "l2tp", "pppossh"}) do

	local proto = netmod:register_protocol(p)

	function proto.get_i18n(self)
		if p == "ppp" then
			return luci.i18n.translate("PPP")
		elseif p == "pptp" then
			return luci.i18n.translate("PPtP")
		elseif p == "3g" then
			return luci.i18n.translate("UMTS/GPRS/EV-DO")
		elseif p == "pppoe" then
			return luci.i18n.translate("PPPoE")
		elseif p == "pppoa" then
			return luci.i18n.translate("PPPoATM")
		elseif p == "l2tp" then
			return luci.i18n.translate("L2TP")
		elseif p == "pppossh" then
			return luci.i18n.translate("PPPoSSH")
		end
	end

	function proto.ifname(self)
		return p .. "-" .. self.sid
	end

	function proto.opkg_package(self)
		if p == "ppp" then
			return p
		elseif p == "3g" then
			return "comgt"
		elseif p == "pptp" then
			return "ppp-mod-pptp"
		elseif p == "pppoe" then
			return "ppp-mod-pppoe"
		elseif p == "pppoa" then
			return "ppp-mod-pppoa"
		elseif p == "l2tp" then
			return "xl2tpd"
		elseif p == "pppossh" then
			return "pppossh"
		end
	end

	function proto.is_installed(self)
		if p == "pppoa" then
			return (nixio.fs.glob("/usr/lib/pppd/*/pppoatm.so")() ~= nil)
		elseif p == "pppoe" then
			return (nixio.fs.glob("/usr/lib/pppd/*/rp-pppoe.so")() ~= nil)
		elseif p == "pptp" then
			return (nixio.fs.glob("/usr/lib/pppd/*/pptp.so")() ~= nil)
		elseif p == "3g" then
			return nixio.fs.access("/lib/netifd/proto/3g.sh")
		elseif p == "l2tp" then
			return nixio.fs.access("/lib/netifd/proto/l2tp.sh")
		elseif p == "pppossh" then
			return nixio.fs.access("/lib/netifd/proto/pppossh.sh")
		else
			return nixio.fs.access("/lib/netifd/proto/ppp.sh")
		end
	end

	function proto.is_floating(self)
		return (p ~= "pppoe")
	end

	function proto.is_virtual(self)
		return true
	end

	function proto.get_interfaces(self)
		if self:is_floating() then
			return nil
		else
			return netmod.protocol.get_interfaces(self)
		end
	end

	function proto.contains_interface(self, ifc)
		if self:is_floating() then
			return (netmod:ifnameof(ifc) == self:ifname())
		else
			return netmod.protocol.contains_interface(self, ifc)
		end
	end

	netmod:register_pattern_virtual("^%s%%-%%w" % p)
end
