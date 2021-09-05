local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname
local has_ss = api.is_finded("ss-redir")
local has_ss_rust = api.is_finded("sslocal")
local has_trojan_plus = api.is_finded("trojan-plus")
local has_v2ray = api.is_finded("v2ray")
local has_xray = api.is_finded("xray")
local has_trojan_go = api.is_finded("trojan-go")
local ss_aead_type = {}
local trojan_type = {}
if has_ss then
    ss_aead_type[#ss_aead_type + 1] = "shadowsocks-libev"
end
if has_ss_rust then
    ss_aead_type[#ss_aead_type + 1] = "shadowsocks-rust"
end
if has_trojan_plus then
    trojan_type[#trojan_type + 1] = "trojan-plus"
end
if has_v2ray then
    trojan_type[#trojan_type + 1] = "v2ray"
    ss_aead_type[#ss_aead_type + 1] = "v2ray"
end
if has_xray then
    trojan_type[#trojan_type + 1] = "xray"
    ss_aead_type[#ss_aead_type + 1] = "xray"
end
if has_trojan_go then
    trojan_type[#trojan_type + 1] = "trojan-go"
end

m = Map(appname)

-- [[ Subscribe Settings ]]--
s = m:section(TypedSection, "global_subscribe", "")
s.anonymous = true

---- Subscribe via proxy
o = s:option(Flag, "subscribe_proxy", translate("Subscribe via proxy"))
o.default = 0
o.rmempty = false

---- Enable auto update subscribe
o = s:option(Flag, "auto_update_subscribe",
             translate("Enable auto update subscribe"))
o.default = 0
o.rmempty = false

---- Week update rules
o = s:option(ListValue, "week_update_subscribe", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update_subscribe", true)

---- Day update rules
o = s:option(ListValue, "time_update_subscribe", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update_subscribe", true)

o = s:option(ListValue, "filter_keyword_mode", translate("Filter keyword Mode"))
o:value("0", translate("Close"))
o:value("1", translate("Discard List"))
o:value("2", translate("Keep List"))

o = s:option(DynamicList, "filter_discard_list", translate("Discard List"))

o = s:option(DynamicList, "filter_keep_list", translate("Keep List"))

o = s:option(Flag, "allowInsecure", translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "1"
o.rmempty = false

if #ss_aead_type > 0 then
    o = s:option(ListValue, "ss_aead_type", translate("SS AEAD Node Use Type"))
    for key, value in pairs(ss_aead_type) do
        o:value(value, translate(value:gsub("^%l",string.upper)))
    end
end

if #trojan_type > 0 then
    o = s:option(ListValue, "trojan_type", translate("Trojan Node Use Type"))
    for key, value in pairs(trojan_type) do
        o:value(value, translate(value:gsub("^%l",string.upper)))
    end
end

---- Manual subscription
o = s:option(Button, "_update", translate("Manual subscription"))
o.inputstyle = "apply"
function o.write(e, e)
    luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua start log > /dev/null 2>&1 &")
    luci.http.redirect(api.url("log"))
end

---- Subscribe Delete All
o = s:option(Button, "_stop", translate("Delete All Subscribe Node"))
o.inputstyle = "remove"
function o.write(e, e)
    luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua truncate > /dev/null 2>&1")
end

s = m:section(TypedSection, "subscribe_list", "",
              "<font color='red'>" .. translate(
                  "Please input the subscription url first, save and submit before updating. If you subscribe to update, it is recommended to delete all subscriptions and then re-subscribe.") ..
                  "</font>")
s.addremove = true
s.anonymous = true
s.sortable = true
s.template = "cbi/tblsection"

o = s:option(Flag, "enabled", translate("Enabled"))
o.default = "1"
o.rmempty = false

o = s:option(Value, "remark", translate("Subscribe Remark"))
o.width = "auto"
o.rmempty = false

o = s:option(DummyValue, "_node_count")
o.rawhtml = true
o.cfgvalue = function(t, n)
    local remark = m:get(n, "remark") or ""
    local num = 0
    m.uci:foreach(appname, "nodes", function(s)
        if s["add_from"] ~= "" and s["add_from"] == remark then
            num = num + 1
        end
    end)
    return string.format("<span title='%s' style='color:red'>%s</span>", remark .. " " .. translate("Node num") .. ": " .. num, num)
end

o = s:option(Value, "url", translate("Subscribe URL"))
o.width = "auto"
o.rmempty = false

o = s:option(Button, "_remove", translate("Delete the subscribed node"))
o.inputstyle = "remove"
function o.write(t, n)
    local remark = m:get(n, "remark") or "" 
    luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua truncate " .. remark .. " > /dev/null 2>&1")
end

return m
