local s = require "luci.sys"
local net = require"luci.model.network".init()
local ifaces = s.net:devices()
local m, s, o

m = Map("pppoe-relay", translate("PPPoE Relay"))
m.description = translate(
                    "Opening the PPPoE relay allows devices in the Intranet to create a separate PPPoE connection that can cross NAT.")

s = m:section(TypedSection, "service")
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:option(DummyValue, "status", translate("Current Condition"))
o.template = "pppoe-relay/status"

o = s:option(ListValue, "server_interface", translate("Server Interface"))
for _, iface in ipairs(ifaces) do
    if not (iface == "lo" or iface:match("^ifb.*") or iface:match("gre*")) then
        local nets = net:get_interface(iface)
        nets = nets and nets:get_networks() or {}
        for k, v in pairs(nets) do nets[k] = nets[k].sid end
        nets = table.concat(nets, ",")
        o:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
    end
end
o.rmempty = true

o = s:option(ListValue, "client_interface", translate("Client Interface"))
for _, iface in ipairs(ifaces) do
    if not (iface == "lo" or iface:match("^ifb.*") or iface:match("gre*")) then
        local nets = net:get_interface(iface)
        nets = nets and nets:get_networks() or {}
        for k, v in pairs(nets) do nets[k] = nets[k].sid end
        nets = table.concat(nets, ",")
        o:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
    end
end
o.rmempty = true

m:append(Template("pppoe-relay/ajax"))

return m
