local sys = require "luci.sys"

m = Map("luci-app-ipsec-server", translate("IPSec VPN Server"))
m.template = "ipsec-server/index"

s = m:section(TypedSection, "service")
s.anonymous = true

o = s:option(DummyValue, "ipsec-server_status", translate("Current Condition"))
o.rawhtml = true
o.cfgvalue = function(t, n)
    return '<font class="ipsec-server_status"></font>'
end

enabled = s:option(Flag, "enabled", translate("Enable"), translate("Use a client that supports IPSec Xauth PSK (iOS or Android) to connect to this server."))
enabled.default = 0
enabled.rmempty = false

clientip = s:option(Value, "clientip", translate("VPN Client IP"))
clientip.datatype = "ip4addr"
clientip.description = translate("VPN Client reserved started IP addresses with the same subnet mask, such as: 192.168.100.10/24")
clientip.optional = false
clientip.rmempty = false

--[[
clientdns = s:option(Value, "clientdns", translate("VPN Client DNS"))
clientdns.datatype = "ip4addr"
clientdns.description = translate("DNS using in VPN tunnel.")
clientdns.optional = false
clientdns.rmempty = false
]]--

secret = s:option(Value, "secret", translate("Secret Pre-Shared Key"))
secret.password = true

if sys.call("command -v xl2tpd > /dev/null") == 0 then
    o = s:option(DummyValue, "l2tp_status", "L2TP " .. translate("Current Condition"))
    o.rawhtml = true
    o.cfgvalue = function(t, n)
        return '<font class="l2tp_status"></font>'
    end

    o = s:option(Flag, "l2tp_enable", "L2TP " .. translate("Enable"), translate("Use a client that supports L2TP over IPSec PSK to connect to this server."))
    o.default = 0
    o.rmempty = false

    o = s:option(Value, "l2tp_localip", "L2TP " .. translate("Server IP"), translate("VPN Server IP address, such as: 192.168.101.1"))
    o.datatype = "ip4addr"
    o.rmempty = true
    o.default = "192.168.101.1"
    o.placeholder = o.default

    o = s:option(Value, "l2tp_remoteip", "L2TP " .. translate("Client IP"), translate("VPN Client IP address range, such as: 192.168.101.10-20"))
    o.rmempty = true
    o.default = "192.168.101.10-20"
    o.placeholder = o.default
end

return m
