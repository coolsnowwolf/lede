local sys = require "luci.sys"
local fs = require "nixio.fs"
local appname = "passwall"

m = Map(appname)

-- [[ Rule List Settings ]]--
s = m:section(TypedSection, "global_rules")
s.anonymous = true

s:tab("direct_list", translate("Direct List"))
s:tab("proxy_list", translate("Proxy List"))

---- Direct Hosts
local direct_host = string.format("/usr/share/%s/rules/direct_host", appname)
o = s:taboption("direct_list", TextValue, "direct_hosts", "", "<font color='red'>" .. translate("Join the direct hosts list of domain names will not proxy.") .. "</font>")
o.rows = 8
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(direct_host) or "" end
o.write = function(self, section, value)
    fs.writefile(direct_host, value:gsub("\r\n", "\n"):gsub("https://", ""):gsub("http://", ""))
end
o.remove = function(self, section, value) fs.writefile(direct_host, "") end

---- Direct IP
local direct_ip = string.format("/usr/share/%s/rules/direct_ip", appname)
o = s:taboption("direct_list", TextValue, "direct_ip", "", "<font color='red'>" .. translate("These had been joined ip addresses will not proxy. Please input the ip address or ip address segment,every line can input only one ip address. For example: 192.168.0.0/24 or 223.5.5.5.") .. "</font>")
o.rows = 8
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(direct_ip) or "" end
o.write = function(self, section, value)
    fs.writefile(direct_ip, value:gsub("\r\n", "\n"):gsub("https://", ""):gsub("http://", ""))
end
o.remove = function(self, section, value) fs.writefile(direct_ip, "") end

---- Proxy Hosts
local proxy_host = string.format("/usr/share/%s/rules/proxy_host", appname)
o = s:taboption("proxy_list", TextValue, "proxy_host", "", "<font color='red'>" .. translate("These had been joined websites will use proxy. Please input the domain names of websites,every line can input only one website domain. For example: google.com.") .. "</font>")
o.rows = 8
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(proxy_host) or "" end
o.write = function(self, section, value)
    fs.writefile(proxy_host, value:gsub("\r\n", "\n"):gsub("https://", ""):gsub("http://", ""))
end
o.remove = function(self, section, value) fs.writefile(proxy_host, "") end

---- Proxy IP
local proxy_ip = string.format("/usr/share/%s/rules/proxy_ip", appname)
o = s:taboption("proxy_list", TextValue, "blacklist_ip", "", "<font color='red'>" .. translate("These had been joined ip addresses will use proxy.Please input the ip address or ip address segment,every line can input only one ip address.For example: 35.24.0.0/24 or 8.8.4.4.") .. "</font>")
o.rows = 8
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(proxy_ip) or "" end
o.write = function(self, section, value)
    fs.writefile(proxy_ip, value:gsub("\r\n", "\n"):gsub("https://", ""):gsub("http://", ""))
end
o.remove = function(self, section, value) fs.writefile(proxy_ip, "") end

-- [[ ACLs Settings ]]--
s = m:section(TypedSection, "acl_rule", translate("ACLs"), "<font color='red'>" .. translate("ACLs is a tools which used to designate specific IP proxy mode, IP or MAC address can be entered.") .. "</font>")
s.template = "cbi/tblsection"
s.sortable = true
s.anonymous = true
s.addremove = true

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.default = 1
o.rmempty = false

---- Remarks
o = s:option(Value, "remarks", translate("Remarks"))
o.rmempty = true

---- IP Address
o = s:option(Value, "ip", translate("IP"))
o.datatype = "ip4addr"
o.rmempty = true

local temp = {}
for index, n in ipairs(luci.ip.neighbors({family = 4})) do
    if n.dest then temp[index] = n.dest:string() end
end
local ips = {}
for _, key in pairs(temp) do table.insert(ips, key) end
table.sort(ips)

for index, key in pairs(ips) do o:value(key, temp[key]) end
-- webadmin.cbi_add_knownips(o)

---- MAC Address
o = s:option(Value, "mac", translate("MAC"))
o.rmempty = true
sys.net.mac_hints(function(e, t) o:value(e, "%s (%s)" % {e, t}) end)

---- TCP Node
local tcp_node_num = m:get("@global_other[0]", "tcp_node_num") or 1
if tonumber(tcp_node_num) > 1 then
    o = s:option(ListValue, "tcp_node", translate("TCP Node"))
    for i = 1, tcp_node_num, 1 do o:value(i, "TCP_" .. i) end
end

---- UDP Node
local udp_node_num = m:get("@global_other[0]", "udp_node_num") or 1
if tonumber(udp_node_num) > 1 then
    o = s:option(ListValue, "udp_node", translate("UDP Node"))
    for i = 1, udp_node_num, 1 do o:value(i, "UDP_" .. i) end
end

---- TCP Proxy Mode
o = s:option(ListValue, "tcp_proxy_mode", "TCP" .. translate("Proxy Mode"))
o.default = "default"
o.rmempty = false
o:value("default", translate("Default"))
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("China WhiteList"))
o:value("returnhome", translate("Return Home"))

---- UDP Proxy Mode
o = s:option(ListValue, "udp_proxy_mode", "UDP" .. translate("Proxy Mode"))
o.default = "default"
o.rmempty = false
o:value("default", translate("Default"))
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode") .. "（" .. translate("China WhiteList") .. "）")
o:value("returnhome", translate("Return Home"))

---- TCP No Redir Ports
o = s:option(Value, "tcp_no_redir_ports", translate("TCP No Redir Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))

---- UDP No Redir Ports
o = s:option(Value, "udp_no_redir_ports", translate("UDP No Redir Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))

---- TCP Redir Ports
o = s:option(Value, "tcp_redir_ports", translate("TCP Redir Ports"))
o.default = "default"
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))
o:value("80,443", "80,443")
o:value("80:", "80 " .. translate("or more"))
o:value(":443", "443 " .. translate("or less"))

---- UDP Redir Ports
o = s:option(Value, "udp_redir_ports", translate("UDP Redir Ports"))
o.default = "default"
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))
o:value("53", "53")

return m
