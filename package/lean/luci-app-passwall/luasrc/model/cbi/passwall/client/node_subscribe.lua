local api = require "luci.passwall.api"
local appname = api.appname
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
api.set_apply_on_parse(m)

-- [[ Subscribe Settings ]]--
s = m:section(TypedSection, "global_subscribe", "")
s.anonymous = true

o = s:option(ListValue, "filter_keyword_mode", translate("Filter keyword Mode"))
o:value("0", translate("Close"))
o:value("1", translate("Discard List"))
o:value("2", translate("Keep List"))
o:value("3", translate("Discard List,But Keep List First"))
o:value("4", translate("Keep List,But Discard List First"))

o = s:option(DynamicList, "filter_discard_list", translate("Discard List"))

o = s:option(DynamicList, "filter_keep_list", translate("Keep List"))

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

---- Subscribe Delete All
o = s:option(Button, "_stop", translate("Delete All Subscribe Node"))
o.inputstyle = "remove"
function o.write(e, e)
	luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua truncate > /dev/null 2>&1")
end

o = s:option(Button, "_update", translate("Manual subscription All"))
o.inputstyle = "apply"
function o.write(t, n)
	luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua start > /dev/null 2>&1 &")
	luci.http.redirect(api.url("log"))
end

s = m:section(TypedSection, "subscribe_list", "", "<font color='red'>" .. translate("Please input the subscription url first, save and submit before manual subscription.") .. "</font>")
s.addremove = true
s.anonymous = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = api.url("node_subscribe_config", "%s")
function s.create(e, t)
	local id = TypedSection.create(e, t)
	luci.http.redirect(e.extedit:format(id))
end

o = s:option(Value, "remark", translate("Remarks"))
o.width = "auto"
o.rmempty = false
o.validate = function(self, value, t)
	if value then
		local count = 0
		m.uci:foreach(appname, "subscribe_list", function(e)
			if e[".name"] ~= t and e["remark"] == value then
				count = count + 1
			end
		end)
		if count > 0 then
			return nil, translate("This remark already exists, please change a new remark.")
		end
		return value
	end
end

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

o = s:option(Button, "_update", translate("Manual subscription"))
o.inputstyle = "apply"
function o.write(t, n)
	luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua start " .. n .. " > /dev/null 2>&1 &")
	luci.http.redirect(api.url("log"))
end

return m
