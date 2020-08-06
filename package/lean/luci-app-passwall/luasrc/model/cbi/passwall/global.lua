local o = require "luci.dispatcher"
local sys = require "luci.sys"
local ipkg = require("luci.model.ipkg")
local uci = require"luci.model.uci".cursor()
local _api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

local function is_installed(e) return ipkg.installed(e) end

local function is_finded(e)
    return
        sys.exec("find /usr/*bin -iname " .. e .. " -type f") ~= "" and true or
            false
end

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

local socks_table = {}
socks_table[#socks_table + 1] = {
    id = "",
    remarks = "127.0.0.1:9050 - dns2sock" .. translate(" Default")
}
uci:foreach(appname, "socks", function(s)
    if s.enabled == "1" and s.node then
        local id, remarks
        local same, i = s.node:match("^(tcp)([1-9])$")
        if same then
            remarks = translatef("Same as the tcp %s node", i)
        else
            for k, n in pairs(nodes_table) do
                if (s.node == n.id) then
                    remarks = n.remarks; break
                end
            end
        end
        id = "0.0.0.0" .. ":" .. s.port
        socks_table[#socks_table + 1] = {
            id = id,
            remarks = id .. " - " .. (remarks or translate("Misconfigured"))
        }
    end
end)

m = Map(appname)
local status_use_big_icon = m:get("@global_other[0]", "status_use_big_icon") or 1
if status_use_big_icon and tonumber(status_use_big_icon) == 1 then
    m:append(Template(appname .. "/global/status"))
else
    m:append(Template(appname .. "/global/status2"))
end

-- [[ Global Settings ]]--
s = m:section(TypedSection, "global")
s.anonymous = true
s.addremove = false

s:tab("Main", translate("Main Settings"))

---- Main switch
o = s:taboption("Main", Flag, "enabled", translate("Main switch"))
o.rmempty = false

---- TCP Node
local tcp_node_num = tonumber(m:get("@global_other[0]", "tcp_node_num") or 1)
for i = 1, tcp_node_num, 1 do
    if i == 1 then
        o = s:taboption("Main", ListValue, "tcp_node" .. i, translate("TCP Node"))
        -- o.description = translate("For used to surf the Internet.")
    else
        o = s:taboption("Main", ListValue, "tcp_node" .. i,
                     translate("TCP Node") .. " " .. i)
    end
    o:value("nil", translate("Close"))
    for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end
end

---- UDP Node
local udp_node_num = tonumber(m:get("@global_other[0]", "udp_node_num") or 1)
for i = 1, udp_node_num, 1 do
    if i == 1 then
        o = s:taboption("Main", ListValue, "udp_node" .. i, translate("UDP Node"))
        -- o.description = translate("For Game Mode or DNS resolution and more.") .. translate("The selected server will not use Kcptun.")
        o:value("nil", translate("Close"))
        o:value("tcp", translate("Same as the tcp node"))
        o:value("tcp_", translate("Same as the tcp node") .. "（" .. translate("New process") .. "）")
    else
        o = s:taboption("Main", ListValue, "udp_node" .. i,
                     translate("UDP Node") .. " " .. i)
        o:value("nil", translate("Close"))
    end
    for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end
end

s:tab("DNS", translate("DNS Settings"))

o = s:taboption("DNS", Value, "up_china_dns", translate("China DNS Server") .. "(UDP)")
-- o.description = translate("If you want to work with other DNS acceleration services, use the default.<br />Only use two at most, english comma separation, If you do not fill in the # and the following port, you are using port 53.")
o.default = "default"
o:value("default", translate("Default"))
o:value("dnsbyisp", translate("dnsbyisp"))
o:value("223.5.5.5", "223.5.5.5 (" .. translate("Ali") .. "DNS)")
o:value("223.6.6.6", "223.6.6.6 (" .. translate("Ali") .. "DNS)")
o:value("114.114.114.114", "114.114.114.114 (114DNS)")
o:value("114.114.115.115", "114.114.115.115 (114DNS)")
o:value("119.29.29.29", "119.29.29.29 (DNSPOD DNS)")
o:value("182.254.116.116", "182.254.116.116 (DNSPOD DNS)")
o:value("1.2.4.8", "1.2.4.8 (CNNIC DNS)")
o:value("210.2.4.8", "210.2.4.8 (CNNIC DNS)")
o:value("180.76.76.76", "180.76.76.76 (" .. translate("Baidu") .. "DNS)")

---- DNS Forward Mode
o = s:taboption("DNS", ListValue, "dns_mode", translate("DNS Mode"))
-- o.description = translate("if has problem, please try another mode.<br />if you use no patterns are used, DNS of wan will be used by default as upstream of dnsmasq.")
o.rmempty = false
o:reset_values()
if is_finded("chinadns-ng") then
    o:value("chinadns-ng", "ChinaDNS-NG")
end
if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o:value("pdnsd", "pdnsd")
end
if is_finded("dns2socks") then
    o:value("dns2socks", "dns2socks")
end
o:value("local_7913", translate("Use local port 7913 as DNS"))
o:value("nonuse", translate("No patterns are used"))

---- Upstream trust DNS Server for ChinaDNS-NG
o = s:taboption("DNS", ListValue, "up_trust_chinadns_ng_dns",
             translate("Upstream trust DNS Server for ChinaDNS-NG") .. "(UDP)")
-- o.description = translate("You can use other resolving DNS services as trusted DNS, Example: dns2socks, dns-forwarder... 127.0.0.1#5353<br />Only use two at most, english comma separation, If you do not fill in the # and the following port, you are using port 53.")
o.default = "pdnsd"
if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o:value("pdnsd", "pdnsd + " .. translate("Use TCP Node Resolve DNS"))
end
if is_finded("dns2socks") then
    o:value("dns2socks", "dns2socks")
end
o:value("udp", translate("Use UDP Node Resolve DNS"))
o:depends("dns_mode", "chinadns-ng")

---- Use TCP Node Resolve DNS
--[[ if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o = s:taboption("DNS", Flag, "use_tcp_node_resolve_dns", translate("Use TCP Node Resolve DNS"))
    o.description = translate("If checked, DNS is resolved using the TCP node.")
    o.default = 1
    o:depends("dns_mode", "pdnsd")
end
--]]

o = s:taboption("DNS", Value, "socks_server", translate("Socks Server"))
o.default = ""
o:depends({dns_mode = "dns2socks"})
o:depends({dns_mode = "chinadns-ng", up_trust_chinadns_ng_dns = "dns2socks"})
for k, v in pairs(socks_table) do o:value(v.id, v.remarks) end

o = s:taboption("DNS", Flag, "fair_mode", translate("Fair Mode"))
o.default = "1"
o:depends({dns_mode = "chinadns-ng"})

---- DNS Forward
o = s:taboption("DNS", Value, "dns_forward", translate("DNS Address"))
o.default = "8.8.4.4"
o:value("8.8.4.4", "8.8.4.4 (Google DNS)")
o:value("8.8.8.8", "8.8.8.8 (Google DNS)")
o:value("208.67.222.222", "208.67.222.222 (Open DNS)")
o:value("208.67.220.220", "208.67.220.220 (Open DNS)")
o:depends({dns_mode = "chinadns-ng"})
o:depends({dns_mode = "dns2socks"})
o:depends({dns_mode = "pdnsd"})

o = s:taboption("DNS", Flag, "dns_cache", translate("DNS Cache"))
o.default = "1"
o:depends({dns_mode = "chinadns-ng", up_trust_chinadns_ng_dns = "pdnsd"})
o:depends({dns_mode = "chinadns-ng", up_trust_chinadns_ng_dns = "dns2socks"})
o:depends({dns_mode = "dns2socks"})
o:depends({dns_mode = "pdnsd"})

s:tab("Mode", translate("Proxy Mode"))

---- TCP Default Proxy Mode
o = s:taboption("Mode", ListValue, "tcp_proxy_mode",
             "TCP" .. translate("Default") .. translate("Proxy Mode"))
-- o.description = translate("If not available, try clearing the cache.")
o.default = "chnroute"
o.rmempty = false
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("China WhiteList"))
o:value("returnhome", translate("Return Home"))

---- UDP Default Proxy Mode
o = s:taboption("Mode", ListValue, "udp_proxy_mode",
             "UDP" .. translate("Default") .. translate("Proxy Mode"))
o.default = "chnroute"
o.rmempty = false
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode") .. "（" .. translate("China WhiteList") .. "）")
o:value("returnhome", translate("Return Home"))

---- Localhost TCP Proxy Mode
o = s:taboption("Mode", ListValue, "localhost_tcp_proxy_mode",
             translate("Router Localhost") .. "TCP" .. translate("Proxy Mode"))
-- o.description = translate("The server client can also use this rule to scientifically surf the Internet.")
o:value("default", translate("Default"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("China WhiteList"))
o:value("global", translate("Global Proxy"))
o.default = "default"
o.rmempty = false

---- Localhost UDP Proxy Mode
o = s:taboption("Mode", ListValue, "localhost_udp_proxy_mode",
             translate("Router Localhost") .. "UDP" .. translate("Proxy Mode"))
o:value("disable", translate("No Proxy"))
o:value("default", translate("Default"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode") .. "（" .. translate("China WhiteList") .. "）")
o:value("global", translate("Global Proxy"))
o.default = "default"
o.rmempty = false

-- [[ Socks Server ]]--
s = m:section(TypedSection, "socks", translate("Socks Config"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
function s.create(e, t)
    TypedSection.create(e, _api.gen_uuid())
end

o = s:option(DummyValue, "status", translate("Status"))
o.template = appname .. "/global/socks_status"

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.default = 1
o.rmempty = false

o = s:option(ListValue, "node", translate("Socks Node"))
local tcp_node_num = tonumber(m:get("@global_other[0]", "tcp_node_num") or 1)
for i = 1, tcp_node_num, 1 do
    o:value("tcp" .. i, translatef("Same as the tcp %s node", i))
end
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end

o = s:option(Value, "port", translate("Listen Port"))
o.datatype = "port"
o.rmempty = false

---- Tips
m:append(Template(appname .. "/global/tips"))

m:append(Template(appname .. "/global/footer"))

--[[
local apply = luci.http.formvalue("cbi.apply")
if apply then
os.execute("/etc/init.d/" .. appname .." restart")
end
--]]

return m
