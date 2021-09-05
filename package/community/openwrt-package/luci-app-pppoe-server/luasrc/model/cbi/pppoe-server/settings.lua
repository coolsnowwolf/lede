local s = require "luci.sys"
local net = require"luci.model.network".init()
local ifaces = s.net:devices()
local m, s, o
m = Map("pppoe-server", translate("PPPoE Server"))
m.description = translate(
                    "The PPPoE server is a broadband access authentication server that prevents ARP spoofing.")
m.template = "pppoe-server/index"
s = m:section(TypedSection, "service")
s.anonymous = true
o = s:option(DummyValue, "rp_pppoe_server_status",
             translate("Current Condition"))
o.template = "pppoe-server/status"
o.value = translate("Collecting data...")
o = s:option(Flag, "enabled", translate("Enable"),
             translate("Enable or disable the PPPoE server"))
o.rmempty = false
o = s:option(ListValue, "client_interface", translate("Client Interface"),
             translate("Listen in this interface"))
for _, iface in ipairs(ifaces) do
    if (iface:match("^br") or iface:match("^eth*") or iface:match("^pppoe*") or iface:match("wlan*")) then
        local nets = net:get_interface(iface)
        nets = nets and nets:get_networks() or {}
        for k, v in pairs(nets) do nets[k] = nets[k].sid end
        nets = table.concat(nets, ",")
        o:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
    end
end
o.rmempty = true

o = s:option(Value, "localip", translate("Server IP"),
             translate("PPPoE Server IP address.default: 10.0.1.1"))
o.datatype = "ipaddr"
o.placeholder = translate("10.0.1.1")
o.rmempty = true
o.default = "10.0.1.1"
o = s:option(Value, "remoteip", translate("Client IP"),
             translate("PPPoE Client IP address.default: 10.0.1.100-254"))
o.placeholder = translate("10.0.1.100-254")
o.rmempty = true
o.default = "10.0.1.100-254"
o = s:option(Value, "count", translate("Client upper limit"), translate(
                 "Specify how many clients can connect to the server simultaneously."))
o.placeholder = translate("50")
o.rmempty = true
o.default = "50"
o = s:option(Value, "dns1", translate("DNS IP address1"), translate(
                 "Set the PPPoE server to default DNS server, which is not required."))
o.placeholder = translate("10.0.1.1")
o.datatype = "ipaddr"
o.default = "10.0.1.1"
o = s:option(Value, "dns2", translate("DNS IP address2"), translate(
                 "Set the PPPoE server to default DNS server, which is not required."))
o.placeholder = translate("119.29.29.29")
o.datatype = "ipaddr"
o.default = "119.29.29.29"
o = s:option(Value, "mru", translate("mru"), translate(
                 "You may not be able to access the Internet if you don't set it up properly.default: 1492"))
o.default = "1492"
o = s:option(Value, "mtu", translate("mtu"), translate(
                 "You may not be able to access the Internet if you don't set it up properly.default: 1492"))
o.default = "1492"
o = s:option(Flag, "is_nat", translate("is_nat"))
o.rmempty = false
o = s:option(ListValue, "export_interface", translate("Interface"),
             translate("Specify interface forwarding traffic."))
o:value("default", translate("default"))
for _, iface in ipairs(ifaces) do
    if (iface:match("^br") or iface:match("^eth*") or iface:match("^pppoe*") or iface:match("wlan*")) then
        local nets = net:get_interface(iface)
        nets = nets and nets:get_networks() or {}
        for k, v in pairs(nets) do nets[k] = nets[k].sid end
        nets = table.concat(nets, ",")
        o:value(iface, ((#nets > 0) and "%s (%s)" % {iface, nets} or iface))
    end
end
o:depends("is_nat", "1")
o = s:option(Value, "log", translate("Log"),
             translate("Log save path, default: /var/log/pppoe-server.log"))
o.default = "/var/log/pppoe-server.log"
o.rmempty = false
return m
