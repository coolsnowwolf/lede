local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname
local has_v2ray = api.is_finded("v2ray")
local has_xray = api.is_finded("xray")

m = Map(appname)
-- [[ Rule Settings ]]--
s = m:section(TypedSection, "global_rules", translate("Rule status"))
s.anonymous = true

--[[
o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false
]]--

---- gfwlist URL
o = s:option(DynamicList, "gfwlist_url", translate("GFW domains(gfwlist) Update URL"))
o:value("https://cdn.jsdelivr.net/gh/YW5vbnltb3Vz/domain-list-community@release/gfwlist.txt", translate("v2fly/domain-list-community"))
o:value("https://cdn.jsdelivr.net/gh/Loyalsoldier/v2ray-rules-dat@release/gfw.txt", translate("Loyalsoldier/v2ray-rules-dat"))
o:value("https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt", translate("Loukky/gfwlist-by-loukky"))
o:value("https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt", translate("gfwlist/gfwlist"))
o.default = "https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt"

----chnroute  URL
o = s:option(DynamicList, "chnroute_url", translate("China IPs(chnroute) Update URL"))
o:value("https://ispip.clang.cn/all_cn.txt", translate("Clang.CN"))
o:value("https://ispip.clang.cn/all_cn_cidr.txt", translate("Clang.CN.CIDR"))
o:value("https://cdn.jsdelivr.net/gh/soffchen/GeoIP2-CN@release/CN-ip-cidr.txt", translate("soffchen/GeoIP2-CN"))
o:value("https://cdn.jsdelivr.net/gh/Hackl0us/GeoIP2-CN@release/CN-ip-cidr.txt", translate("Hackl0us/GeoIP2-CN"))
o.default = "https://ispip.clang.cn/all_cn.txt"

----chnroute6 URL
o = s:option(DynamicList, "chnroute6_url", translate("China IPv6s(chnroute6) Update URL"))
o:value("https://ispip.clang.cn/all_cn_ipv6.txt", translate("Clang.CN.IPv6"))
o.default = "https://ispip.clang.cn/all_cn_ipv6.txt"

----chnlist URL
o = s:option(DynamicList, "chnlist_url", translate("China List(Chnlist) Update URL"))
o:value("https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/accelerated-domains.china.conf", translate("felixonmars/domains.china"))
o:value("https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/apple.china.conf", translate("felixonmars/apple.china"))
o:value("https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/google.china.conf", translate("felixonmars/google.china"))

s:append(Template(appname .. "/rule/rule_version"))

---- Auto Update
o = s:option(Flag, "auto_update", translate("Enable auto update rules"))
o.default = 0
o.rmempty = false

---- Week Update
o = s:option(ListValue, "week_update", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update", true)

---- Time Update
o = s:option(ListValue, "time_update", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update", true)

if has_v2ray or has_xray then
    o = s:option(Value, "v2ray_location_asset", translate("Location of V2ray/Xray asset"), translate("This variable specifies a directory where geoip.dat and geosite.dat files are."))
    o.default = "/usr/share/xray/"
    o.rmempty = false
    
    s = m:section(TypedSection, "shunt_rules", "V2ray/Xray " .. translate("Shunt Rule"), "<a style='color: red'>" .. translate("Please note attention to the priority, the higher the order, the higher the priority.") .. "</a>")
    s.template = "cbi/tblsection"
    s.anonymous = false
    s.addremove = true
    s.sortable = true
    s.extedit = api.url("shunt_rules", "%s")
    function s.create(e, t)
        TypedSection.create(e, t)
        luci.http.redirect(e.extedit:format(t))
    end
    function s.remove(e, t)
        m.uci:foreach(appname, "nodes", function(s)
            if s["protocol"] and s["protocol"] == "_shunt" then
                m:del(s[".name"], t)
            end
        end)
        TypedSection.remove(e, t)
    end

    o = s:option(DummyValue, "remarks", translate("Remarks"))
end

return m
