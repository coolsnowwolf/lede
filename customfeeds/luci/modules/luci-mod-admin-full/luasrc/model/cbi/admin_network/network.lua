-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local fs = require "nixio.fs"

m = Map("network", translate("Interfaces"))
m.pageaction = false
m:section(SimpleSection).template = "admin_network/iface_overview"

if fs.access("/etc/init.d/dsl_control") then
	dsl = m:section(TypedSection, "dsl", translate("DSL"))

	dsl.anonymous = true

	annex = dsl:option(ListValue, "annex", translate("Annex"))
	annex:value("a", translate("Annex A + L + M (all)"))
	annex:value("b", translate("Annex B (all)"))
	annex:value("j", translate("Annex J (all)"))
	annex:value("m", translate("Annex M (all)"))
	annex:value("bdmt", translate("Annex B G.992.1"))
	annex:value("b2", translate("Annex B G.992.3"))
	annex:value("b2p", translate("Annex B G.992.5"))
	annex:value("at1", translate("ANSI T1.413"))
	annex:value("admt", translate("Annex A G.992.1"))
	annex:value("alite", translate("Annex A G.992.2"))
	annex:value("a2", translate("Annex A G.992.3"))
	annex:value("a2p", translate("Annex A G.992.5"))
	annex:value("l", translate("Annex L G.992.3 POTS 1"))
	annex:value("m2", translate("Annex M G.992.3"))
	annex:value("m2p", translate("Annex M G.992.5"))

	tone = dsl:option(ListValue, "tone", translate("Tone"))
	tone:value("", translate("auto"))
	tone:value("a", translate("A43C + J43 + A43"))
	tone:value("av", translate("A43C + J43 + A43 + V43"))
	tone:value("b", translate("B43 + B43C"))
	tone:value("bv", translate("B43 + B43C + V43"))

	xfer_mode = dsl:option(ListValue, "xfer_mode", translate("Encapsulation mode"))
	xfer_mode:value("", translate("auto"))
	xfer_mode:value("atm", translate("ATM (Asynchronous Transfer Mode)"))
	xfer_mode:value("ptm", translate("PTM/EFM (Packet Transfer Mode)"))

	line_mode = dsl:option(ListValue, "line_mode", translate("DSL line mode"))
	line_mode:value("", translate("auto"))
	line_mode:value("adsl", translate("ADSL"))
	line_mode:value("vdsl", translate("VDSL"))

	firmware = dsl:option(Value, "firmware", translate("Firmware File"))

	m.pageaction = true
end

-- Show ATM bridge section if we have the capabilities
if fs.access("/usr/sbin/br2684ctl") then
	atm = m:section(TypedSection, "atm-bridge", translate("ATM Bridges"),
		translate("ATM bridges expose encapsulated ethernet in AAL5 " ..
			"connections as virtual Linux network interfaces which can " ..
			"be used in conjunction with DHCP or PPP to dial into the " ..
			"provider network."))

	atm.addremove = true
	atm.anonymous = true

	atm.create = function(self, section)
		local sid = TypedSection.create(self, section)
		local max_unit = -1

		m.uci:foreach("network", "atm-bridge",
			function(s)
				local u = tonumber(s.unit)
				if u ~= nil and u > max_unit then
					max_unit = u
				end
			end)

		m.uci:set("network", sid, "unit", max_unit + 1)
		m.uci:set("network", sid, "atmdev", 0)
		m.uci:set("network", sid, "encaps", "llc")
		m.uci:set("network", sid, "payload", "bridged")
		m.uci:set("network", sid, "vci", 35)
		m.uci:set("network", sid, "vpi", 8)

		return sid
	end

	atm:tab("general", translate("General Setup"))
	atm:tab("advanced", translate("Advanced Settings"))

	vci    = atm:taboption("general", Value, "vci", translate("ATM Virtual Channel Identifier (VCI)"))
	vpi    = atm:taboption("general", Value, "vpi", translate("ATM Virtual Path Identifier (VPI)"))
	encaps = atm:taboption("general", ListValue, "encaps", translate("Encapsulation mode"))
	encaps:value("llc", translate("LLC"))
	encaps:value("vc", translate("VC-Mux"))

	atmdev  = atm:taboption("advanced", Value, "atmdev", translate("ATM device number"))
	unit    = atm:taboption("advanced", Value, "unit", translate("Bridge unit number"))
	payload = atm:taboption("advanced", ListValue, "payload", translate("Forwarding mode"))
	payload:value("bridged", translate("bridged"))
	payload:value("routed", translate("routed"))
	m.pageaction = true
end

local network = require "luci.model.network"
if network:has_ipv6() then
	local s = m:section(NamedSection, "globals", "globals", translate("Global network options"))
	local o = s:option(Value, "ula_prefix", translate("IPv6 ULA-Prefix"))
	o.datatype = "ip6addr"
	o.rmempty = true
	m.pageaction = true
end


return m
