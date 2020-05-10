local o = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local ipkg = require("luci.model.ipkg")
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

local function is_installed(e) return ipkg.installed(e) end

local function is_finded(e)
    return
        sys.exec("find /usr/*bin -iname " .. e .. " -type f") ~= "" and true or
            false
end

local n = {}
uci:foreach(appname, "nodes", function(e)
    local type = e.type
    if type == nil then type = "" end
    local address = e.address
    if address == nil then address = "" end
    -- if (type == "V2ray_balancing" or type == "V2ray_shunt") or (address:match("[\u4e00-\u9fa5]") and address:find("%.") and address:sub(#address) ~= ".") then
    if type and address and e.remarks then
        if e.use_kcp and e.use_kcp == "1" then
            n[e[".name"]] = "%s+%s：[%s] %s" %
                                {translate(type), "Kcptun", e.remarks, address}
        else
            n[e[".name"]] = "%s：[%s] %s" %
                                {translate(type), e.remarks, address}
        end
    end
    -- end
end)

local key_table = {}
for key, _ in pairs(n) do table.insert(key_table, key) end
table.sort(key_table)

m = Map(appname)
local status_use_big_icon = api.uci_get_type("global_other",
                                             "status_use_big_icon", 1)
if status_use_big_icon and tonumber(status_use_big_icon) == 1 then
    m:append(Template("passwall/global/status"))
else
    m:append(Template("passwall/global/status2"))
end

-- [[ Global Settings ]]--
s = m:section(TypedSection, "global", translate("Global Settings"))
-- s.description = translate("If you can use it, very stable. If not, GG !!!")
s.anonymous = true
s.addremove = false

---- Main switch
o = s:option(Flag, "enabled", translate("Main switch"))
o.rmempty = false

---- TCP Node
local tcp_node_num = tonumber(
                         api.uci_get_type("global_other", "tcp_node_num", 1))
for i = 1, tcp_node_num, 1 do
    if i == 1 then
        o = s:option(ListValue, "tcp_node" .. i, translate("TCP Node"))
        -- o.description = translate("For used to surf the Internet.")
    else
        o = s:option(ListValue, "tcp_node" .. i,
                     translate("TCP Node") .. " " .. i)
    end
    o:value("nil", translate("Close"))
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

---- UDP Node
local udp_node_num = tonumber(
                         api.uci_get_type("global_other", "udp_node_num", 1))
for i = 1, udp_node_num, 1 do
    if i == 1 then
        o = s:option(ListValue, "udp_node" .. i, translate("UDP Node"))
        -- o.description = translate("For Game Mode or DNS resolution and more.") .. translate("The selected server will not use Kcptun.")
        o:value("nil", translate("Close"))
        o:value("tcp", translate("Same as the tcp node"))
    else
        o = s:option(ListValue, "udp_node" .. i,
                     translate("UDP Node") .. " " .. i)
        o:value("nil", translate("Close"))
    end
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

---- Socks Node
local socks_node_num = tonumber(api.uci_get_type("global_other",
                                                  "socks_node_num", 1))
for i = 1, socks_node_num, 1 do
    if i == 1 then
        o = s:option(ListValue, "socks_node" .. i, translate("Socks Node"))
        -- o.description = translate("The client can use the router's Socks proxy.")
        o:value("nil", translate("Close"))
        o:value("tcp", translate("Same as the tcp node"))
    else
        o = s:option(ListValue, "socks_node" .. i,
                     translate("Socks Node") .. " " .. i)
        o:value("nil", translate("Close"))
    end
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

o = s:option(Value, "up_china_dns", translate("China DNS Server") .. "(UDP)")
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
o = s:option(ListValue, "dns_mode", translate("DNS Mode"))
-- o.description = translate("if has problem, please try another mode.<br />if you use no patterns are used, DNS of wan will be used by default as upstream of dnsmasq.")
o.rmempty = false
o:reset_values()
if is_finded("chinadns-ng") then o:value("chinadns-ng", "ChinaDNS-NG") end
if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o:value("pdnsd", "pdnsd")
end
if is_finded("dns2socks") then
    o:value("dns2socks",
            "dns2socks + " .. translate("Use Socks Node Resolve DNS"))
end
o:value("local_7913", translate("Use local port 7913 as DNS"))
o:value("nonuse", translate("No patterns are used"))

---- Upstream trust DNS Server for ChinaDNS-NG
o = s:option(Value, "up_trust_chinadns_ng_dns",
             translate("Upstream trust DNS Server for ChinaDNS-NG") .. "(UDP)")
-- o.description = translate("You can use other resolving DNS services as trusted DNS, Example: dns2socks, dns-forwarder... 127.0.0.1#5353<br />Only use two at most, english comma separation, If you do not fill in the # and the following port, you are using port 53.")
o.default = "pdnsd"
if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o:value("pdnsd", "pdnsd + " .. translate("Use TCP Node Resolve DNS"))
end
if is_finded("dns2socks") then
    o:value("dns2socks",
            "dns2socks + " .. translate("Use Socks Node Resolve DNS"))
end
o:value("8.8.4.4,8.8.8.8", "8.8.4.4, 8.8.8.8 (Google DNS)")
o:value("208.67.222.222,208.67.220.220",
        "208.67.222.222, 208.67.220.220 (Open DNS)")
o:depends("dns_mode", "chinadns-ng")

---- Use TCP Node Resolve DNS
--[[ if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o = s:option(Flag, "use_tcp_node_resolve_dns",
                 translate("Use TCP Node Resolve DNS"))
-- o.description = translate("If checked, DNS is resolved using the TCP node.")
    o.default = 1
    o:depends("dns_mode", "pdnsd")
end
--]]

o = s:option(Value, "dns2socks_forward", translate("DNS Address"))
o.default = "8.8.4.4"
o:value("8.8.4.4", "8.8.4.4 (Google DNS)")
o:value("8.8.8.8", "8.8.8.8 (Google DNS)")
o:value("208.67.222.222", "208.67.222.222 (Open DNS)")
o:value("208.67.220.220", "208.67.220.220 (Open DNS)")
o:depends("dns_mode", "dns2socks")
o:depends("up_trust_chinadns_ng_dns", "dns2socks")

---- DNS Forward
o = s:option(Value, "dns_forward", translate("DNS Address"))
o.default = "8.8.4.4, 8.8.8.8"
o:value("8.8.4.4, 8.8.8.8", "8.8.4.4, 8.8.8.8 (Google DNS)")
o:value("208.67.222.222", "208.67.222.222 (Open DNS)")
o:value("208.67.220.220", "208.67.220.220 (Open DNS)")
o:depends("dns_mode", "pdnsd")
o:depends("up_trust_chinadns_ng_dns", "pdnsd")

---- TCP Default Proxy Mode
o = s:option(ListValue, "tcp_proxy_mode",
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
o = s:option(ListValue, "udp_proxy_mode",
             "UDP" .. translate("Default") .. translate("Proxy Mode"))
o.default = "chnroute"
o.rmempty = false
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode") .. "（" .. translate("China WhiteList") .. "）")
o:value("returnhome", translate("Return Home"))

---- Localhost TCP Proxy Mode
o = s:option(ListValue, "localhost_tcp_proxy_mode",
             translate("Router Localhost") .. "TCP" .. translate("Proxy Mode"))
-- o.description = translate("The server client can also use this rule to scientifically surf the Internet.")
o:value("default", translate("Default"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("China WhiteList"))
o:value("global", translate("Global Proxy"))
o.default = "default"
o.rmempty = false

---- Localhost UDP Proxy Mode
o = s:option(ListValue, "localhost_udp_proxy_mode",
             translate("Router Localhost") .. "UDP" .. translate("Proxy Mode"))
o:value("disable", translate("No Proxy"))
o:value("default", translate("Default"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode") .. "（" .. translate("China WhiteList") .. "）")
o:value("global", translate("Global Proxy"))
o.default = "default"
o.rmempty = false

---- Tips
s:append(Template("passwall/global/tips"))

--[[
local apply = luci.http.formvalue("cbi.apply")
if apply then
os.execute("/etc/init.d/passwall restart")
end
--]]

return m
