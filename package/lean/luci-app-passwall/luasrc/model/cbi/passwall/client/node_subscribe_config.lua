local api = require "luci.passwall.api"
local appname = api.appname
local sys = api.sys
local has_ss = api.is_finded("ss-redir")
local has_ss_rust = api.is_finded("sslocal")
local has_trojan_plus = api.is_finded("trojan-plus")
local has_singbox = api.finded_com("singbox")
local has_xray = api.finded_com("xray")
local has_trojan_go = api.finded_com("trojan-go")
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
if has_singbox then
	trojan_type[#trojan_type + 1] = "sing-box"
	ss_aead_type[#ss_aead_type + 1] = "sing-box"
end
if has_xray then
	trojan_type[#trojan_type + 1] = "xray"
	ss_aead_type[#ss_aead_type + 1] = "xray"
end
if has_trojan_go then
	trojan_type[#trojan_type + 1] = "trojan-go"
end

m = Map(appname)
m.redirect = api.url("node_subscribe")
api.set_apply_on_parse(m)

s = m:section(NamedSection, arg[1])
s.addremove = false
s.dynamic = false

o = s:option(Value, "remark", translate("Subscribe Remark"))
o.rmempty = false

o = s:option(TextValue, "url", translate("Subscribe URL"))
o.rows = 5
o.rmempty = false

o = s:option(Flag, "allowInsecure", translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "0"
o.rmempty = false

o = s:option(ListValue, "filter_keyword_mode", translate("Filter keyword Mode"))
o.default = "5"
o:value("0", translate("Close"))
o:value("1", translate("Discard List"))
o:value("2", translate("Keep List"))
o:value("3", translate("Discard List,But Keep List First"))
o:value("4", translate("Keep List,But Discard List First"))
o:value("5", translate("Use global config"))

o = s:option(DynamicList, "filter_discard_list", translate("Discard List"))
o:depends("filter_keyword_mode", "1")
o:depends("filter_keyword_mode", "3")
o:depends("filter_keyword_mode", "4")

o = s:option(DynamicList, "filter_keep_list", translate("Keep List"))
o:depends("filter_keyword_mode", "2")
o:depends("filter_keyword_mode", "3")
o:depends("filter_keyword_mode", "4")

if #ss_aead_type > 0 then
	o = s:option(ListValue, "ss_aead_type", translate("SS AEAD Node Use Type"))
	o.default = "global"
	o:value("global", translate("Use global config"))
	for key, value in pairs(ss_aead_type) do
		o:value(value, translate(value:gsub("^%l",string.upper)))
	end
end

if #trojan_type > 0 then
	o = s:option(ListValue, "trojan_type", translate("Trojan Node Use Type"))
	o.default = "global"
	o:value("global", translate("Use global config"))
	for key, value in pairs(trojan_type) do
		o:value(value, translate(value:gsub("^%l",string.upper)))
	end
end

---- Enable auto update subscribe
o = s:option(Flag, "auto_update", translate("Enable auto update subscribe"))
o.default = 0
o.rmempty = false

---- Week update rules
o = s:option(ListValue, "week_update", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update", true)

---- Day update rules
o = s:option(ListValue, "time_update", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update", true)

o = s:option(Value, "user_agent", translate("User-Agent"))
o.default = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.122 Safari/537.36"

return m
