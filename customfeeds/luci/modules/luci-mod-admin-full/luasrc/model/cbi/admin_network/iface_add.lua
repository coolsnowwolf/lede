-- Copyright 2009-2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local nw  = require "luci.model.network".init()
local fw  = require "luci.model.firewall".init()
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()

m = SimpleForm("network", translate("Create Interface"))
m.redirect = luci.dispatcher.build_url("admin/network/network")
m.reset = false

newnet = m:field(Value, "_netname", translate("Name of the new interface"),
	translate("The allowed characters are: <code>A-Z</code>, <code>a-z</code>, " ..
		"<code>0-9</code> and <code>_</code>"
	))

newnet:depends("_attach", "")
newnet.default = arg[1] and "net_" .. arg[1]:gsub("[^%w_]+", "_")
newnet.datatype = "and(uciname,maxlength(15))"

advice = m:field(DummyValue, "d1", translate("Note: interface name length"),
	 translate("Maximum length of the name is 15 characters including " ..
		"the automatic protocol/bridge prefix (br-, 6in4-, pppoe- etc.)"
	))

newproto = m:field(ListValue, "_netproto", translate("Protocol of the new interface"))

netbridge = m:field(Flag, "_bridge", translate("Create a bridge over multiple interfaces"))


sifname = m:field(Value, "_ifname", translate("Cover the following interface"))

sifname.widget = "radio"
sifname.template  = "cbi/network_ifacelist"
sifname.nobridges = true


mifname = m:field(Value, "_ifnames", translate("Cover the following interfaces"))

mifname.widget = "checkbox"
mifname.template  = "cbi/network_ifacelist"
mifname.nobridges = true


local _, p
for _, p in ipairs(nw:get_protocols()) do
	if p:is_installed() then
		newproto:value(p:proto(), p:get_i18n())
		if not p:is_virtual()  then netbridge:depends("_netproto", p:proto()) end
		if not p:is_floating() then
			sifname:depends({ _bridge = "",  _netproto = p:proto()})
			mifname:depends({ _bridge = "1", _netproto = p:proto()})
		end
	end
end

function newproto.validate(self, value, section)
	local name = newnet:formvalue(section)
	if not name or #name == 0 then
		newnet:add_error(section, translate("No network name specified"))
	elseif m:get(name) then
		newnet:add_error(section, translate("The given network name is not unique"))
	end

	local proto = nw:get_protocol(value)
	if proto and not proto:is_floating() then
		local br = (netbridge:formvalue(section) == "1")
		local ifn = br and mifname:formvalue(section) or sifname:formvalue(section)
		for ifn in utl.imatch(ifn) do
			return value
		end
		return nil, translate("The selected protocol needs a device assigned")
	end
	return value
end

function newproto.write(self, section, value)
	local name = newnet:formvalue(section)
	if name and #name > 0 then
		local br = (netbridge:formvalue(section) == "1") and "bridge" or nil
		local net = nw:add_network(name, { proto = value, type = br })
		if net then
			local ifn
			for ifn in utl.imatch(
				br and mifname:formvalue(section) or sifname:formvalue(section)
			) do
				net:add_interface(ifn)
			end
			nw:save("network")
			nw:save("wireless")
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/network/network", name))
	end
end

return m
