local uci = require"luci.model.uci".cursor()
local appname = "passwall"

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
o:depends("auto_on", "1")
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end

---- Automatically turn on time
o = s:option(ListValue, "time_on", translate("Automatically turn on time"))
o.default = nil
o:depends("auto_on", "1")
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end

---- Automatically restart time
o = s:option(ListValue, "time_restart", translate("Automatically restart time"))
o.default = nil
o:depends("auto_on", "1")
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

---- Multi SS/SSR Process Option
o = s:option(Value, "process", translate("Multi Process Option"))
o.default = "0"
o.rmempty = false
o:value("0", translate("Auto"))
o:value("1", translate("1 Process"))
o:value("2", "2 " .. translate("Process"))
o:value("3", "3 " .. translate("Process"))
o:value("4", "4 " .. translate("Process"))

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
s = m:section(TypedSection, "global_other", translate("Other Settings"),
              "<font color='red'>" .. translatef(
                  "You can only set up a maximum of %s nodes for the time being, Used for access control.",
                  "3") .. "</font>")
s.anonymous = true
s.addremove = false

---- TCP Node Number Option
o = s:option(ListValue, "tcp_node_num", "TCP" .. translate("Node Number"))
o.default = "1"
o.rmempty = false
o:value("1")
o:value("2")
o:value("3")

---- UDP Node Number Option
o = s:option(ListValue, "udp_node_num", "UDP" .. translate("Node Number"))
o.default = "1"
o.rmempty = false
o:value("1")
o:value("2")
o:value("3")

---- 状态使用大图标
o = s:option(Flag, "status_use_big_icon", translate("Status Use Big Icon"))
o.default = "1"
o.rmempty = false

---- 显示节点检测
o =
    s:option(Flag, "status_show_check_port", translate("Status Show Check Port"))
o.default = "0"
o.rmempty = false

---- 显示IP111
o = s:option(Flag, "status_show_ip111", translate("Status Show IP111"))
o.default = "0"
o.rmempty = false

local nodes_table = {}
uci:foreach(appname, "nodes", function(e)
    if e.type and e.remarks then
        local remarks = ""
        if e.type == "V2ray" and (e.protocol == "_balancing" or e.protocol == "_shunt") then
            remarks = "%s：[%s] " % {translatef(e.type .. e.protocol), e.remarks}
        else
            if e.use_kcp and e.use_kcp == "1" then
                remarks = "%s+%s：[%s] %s" % {e.type, "Kcptun", e.remarks, e.address}
            else
                remarks = "%s：[%s] %s:%s" % {e.type, e.remarks, e.address, e.port}
            end
        end
        nodes_table[#nodes_table + 1] = {
            id = e[".name"],
            remarks = remarks
         }
    end
end)

return m
