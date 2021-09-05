local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

m = Map(appname)

-- [[ Delay Settings ]]--
s = m:section(TypedSection, "global_delay", translate("Delay Settings"))
s.anonymous = true
s.addremove = false

---- Delay Start
o = s:option(Value, "start_delay", translate("Delay Start"),
             translate("Units:seconds"))
o.default = "1"
o.rmempty = true

---- Open and close Daemon
o = s:option(Flag, "start_daemon", translate("Open and close Daemon"))
o.default = 1
o.rmempty = false

--[[
---- Open and close automatically
o = s:option(Flag, "auto_on", translate("Open and close automatically"))
o.default = 0
o.rmempty = false

---- Automatically turn off time
o = s:option(ListValue, "time_off", translate("Automatically turn off time"))
o.default = nil
o:depends("auto_on", true)
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end

---- Automatically turn on time
o = s:option(ListValue, "time_on", translate("Automatically turn on time"))
o.default = nil
o:depends("auto_on", true)
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end

---- Automatically restart time
o = s:option(ListValue, "time_restart", translate("Automatically restart time"))
o.default = nil
o:depends("auto_on", true)
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
--]]

-- [[ Forwarding Settings ]]--
s = m:section(TypedSection, "global_forwarding",
              translate("Forwarding Settings"))
s.anonymous = true
s.addremove = false

---- TCP No Redir Ports
o = s:option(Value, "tcp_no_redir_ports", translate("TCP No Redir Ports"))
o.default = "disable"
o:value("disable", translate("No patterns are used"))
o:value("1:65535", translate("All"))

---- UDP No Redir Ports
o = s:option(Value, "udp_no_redir_ports", translate("UDP No Redir Ports"),
             "<font color='red'>" .. translate(
                 "Fill in the ports you don't want to be forwarded by the agent, with the highest priority.") ..
                 "</font>")
o.default = "disable"
o:value("disable", translate("No patterns are used"))
o:value("1:65535", translate("All"))

---- TCP Redir Ports
o = s:option(Value, "tcp_redir_ports", translate("TCP Redir Ports"))
o.default = "22,25,53,143,465,587,993,995,80,443"
o:value("1:65535", translate("All"))
o:value("22,25,53,143,465,587,993,995,80,443", translate("Common Use"))
o:value("80,443", translate("Only Web"))
o:value("80:65535", "80 " .. translate("or more"))
o:value("1:443", "443 " .. translate("or less"))

---- UDP Redir Ports
o = s:option(Value, "udp_redir_ports", translate("UDP Redir Ports"))
o.default = "1:65535"
o:value("1:65535", translate("All"))
o:value("53", "DNS")

o = s:option(Flag, "accept_icmp", translate("Hijacking ICMP (PING)"))
o.default = 0

if os.execute("lsmod | grep -i REDIRECT >/dev/null") == 0 and os.execute("lsmod | grep -i TPROXY >/dev/null") == 0 then
    o = s:option(ListValue, "tcp_proxy_way", translate("TCP Proxy Way"))
    o.default = "redirect"
    o:value("redirect", "REDIRECT")
    o:value("tproxy", "TPROXY")
end

--[[
---- Proxy IPv6
o = s:option(Flag, "proxy_ipv6", translate("Proxy IPv6"),
             translate("The IPv6 traffic can be proxyed when selected"))
o.default = 0
--]]

--[[
---- TCP Redir Port
o = s:option(Value, "tcp_redir_port", translate("TCP Redir Port"))
o.datatype = "port"
o.default = 1041
o.rmempty = true

---- UDP Redir Port
o = s:option(Value, "udp_redir_port", translate("UDP Redir Port"))
o.datatype = "port"
o.default = 1051
o.rmempty = true

---- Kcptun Port
o = s:option(Value, "kcptun_port", translate("Kcptun Port"))
o.datatype = "port"
o.default = 12948
o.rmempty = true
--]]

-- [[ Other Settings ]]--
s = m:section(TypedSection, "global_other", translate("Other Settings"))
s.anonymous = true
s.addremove = false

---- IPv6 TProxy
o = s:option(Flag, "ipv6_tproxy", translate("IPv6 TProxy"),
             "<font color='red'>" .. translate(
                 "Experimental feature.Make sure that your node supports IPv6.") ..
                 "</font>")
o.default = 0
o.rmempty = false

o = s:option(MultiValue, "status", translate("Status info"))
o:value("big_icon", translate("Big icon")) -- 大图标
o:value("show_check_port", translate("Show node check")) -- 显示节点检测
o:value("show_ip111", translate("Show Show IP111")) -- 显示IP111

return m
