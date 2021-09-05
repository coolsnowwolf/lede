
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local sys = require "luci.sys"
local json = require "luci.jsonc"
local sid = arg[1]

font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]


m = Map(openclash, translate("Config Subscribe Edit"))
m.pageaction = false
m.description=translate("Convert Subscribe function of Online is Supported By subconverter Written By tindy X") ..
"<br/>"..
"<br/>"..translate("API By tindy X & lhie1")..
"<br/>"..
"<br/>"..translate("Subconverter external configuration (subscription conversion template) Description: https://github.com/tindy2013/subconverter#external-configuration-file")..
"<br/>"..
"<br/>"..translate("If you need to customize the external configuration file (subscription conversion template), please write it according to the instructions, upload it to the accessible location of the external network, and fill in the address correctly when using it")..
"<br/>"..
"<br/>"..translate("If you have a recommended external configuration file (subscription conversion template), you can modify by following The file format of /usr/share/opencrash/res/sub_ini.list and pr")
m.redirect = luci.dispatcher.build_url("admin/services/openclash/config-subscribe")
if m.uci:get(openclash, sid) ~= "config_subscribe" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Config Subscribe Setting ]]--
s = m:section(NamedSection, sid, "config_subscribe")
s.anonymous = true
s.addremove   = false

---- name
o = s:option(Value, "name", translate("Config Alias"))
o.description = font_red..bold_on..translate("Name For Distinguishing")..bold_off..font_off
o.placeholder = translate("config")
o.rmempty = true

---- address
o = s:option(Value, "address", translate("Subscribe Address"))
o.description = font_red..bold_on..translate("Not Null")..bold_off..font_off
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o.rmempty = false

local sub_path = "/tmp/dler_sub"
local info, token, get_sub, sub_info
local token = uci:get("openclash", "config", "dler_token")
if token then
	get_sub = string.format("curl -sL -H 'Content-Type: application/json' --connect-timeout 2 -d '{\"access_token\":\"%s\"}' -X POST https://dler.cloud/api/v1/managed/clash -o %s", token, sub_path)
	if not nixio.fs.access(sub_path) then
		luci.sys.exec(get_sub)
	else
		if fs.readfile(sub_path) == "" or not fs.readfile(sub_path) then
			luci.sys.exec(get_sub)
		end
	end
	sub_info = fs.readfile(sub_path)
	if sub_info then
		sub_info = json.parse(sub_info)
	end
	if sub_info and sub_info.ret == 200 then
		o:value(sub_info.smart)
		o:value(sub_info.ss)
		o:value(sub_info.vmess)
		o:value(sub_info.trojan)
	else
		fs.unlink(sub_path)
	end
end
	
---- subconverter
o = s:option(Flag, "sub_convert", translate("Subscribe Convert Online"))
o.description = translate("Convert Subscribe Online With Template, Mix Proxies and Keep Settings options Will Not Effect")
o.default=0

---- Convert Address
o = s:option(Value, "convert_address", translate("Convert Address"))
o.rmempty     = true
o.description = font_red..bold_on..translate("Note: There is A Risk of Privacy Leakage in Online Convert")..bold_off..font_off
o:depends("sub_convert", "1")
o:value("https://api.dler.io/sub", translate("api.dler.io")..translate("(Default)"))
o:value("https://subcon.dlj.tf/sub", translate("subcon.dlj.tf")..translate("(Default)"))
o:value("https://subconverter-web.now.sh/sub", translate("subconverter-web.now.sh"))
o:value("https://subconverter.herokuapp.com/sub", translate("subconverter.herokuapp.com"))
o:value("https://sub.id9.cc/sub", translate("sub.id9.cc"))
o:value("https://api.wcc.best/sub", translate("api.wcc.best"))
o.default = "https://api.dler.io/sub"

---- Template
o = s:option(ListValue, "template", translate("Template Name"))
o.rmempty     = true
o:depends("sub_convert", "1")
file = io.open("/usr/share/openclash/res/sub_ini.list", "r");
for l in file:lines() do
	if l ~= "" and l ~= nil then
		o:value(string.sub(luci.sys.exec(string.format("echo '%s' |awk -F ',' '{print $1}' 2>/dev/null",l)),1,-2))
	end
end
file:close()
o:value("0", translate("Custom Template"))

---- Custom Template
o = s:option(Value, "custom_template_url", translate("Custom Template URL"))
o.rmempty     = true
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o:depends("template", "0")

---- emoji
o = s:option(ListValue, "emoji", translate("Emoji"))
o.rmempty     = false
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=0
o:depends("sub_convert", "1")

---- udp
o = s:option(ListValue, "udp", translate("UDP Enable"))
o.rmempty     = false
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=0
o:depends("sub_convert", "1")

---- skip-cert-verify
o = s:option(ListValue, "skip_cert_verify", translate("skip-cert-verify"))
o.rmempty     = false
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=0
o:depends("sub_convert", "1")

---- sort
o = s:option(ListValue, "sort", translate("Sort"))
o.rmempty     = false
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=0
o:depends("sub_convert", "1")

---- node type
o = s:option(ListValue, "node_type", translate("Append Node Type"))
o.rmempty     = false
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=0
o:depends("sub_convert", "1")

---- key
o = s:option(DynamicList, "keyword", font_red..bold_on..translate("Keyword Match")..bold_off..font_off)
o.description = font_red..bold_on..translate("eg: hk or tw&bgp")..bold_off..font_off
o.rmempty = true

---- exkey
o = s:option(DynamicList, "ex_keyword", font_red..bold_on..translate("Exclude Keyword Match")..bold_off..font_off)
o.description = font_red..bold_on..translate("eg: hk or tw&bgp")..bold_off..font_off
o.rmempty = true

---- de_exkey
o = s:option(MultiValue, "de_ex_keyword", font_red..bold_on..translate("Exclude Keyword Match Default")..bold_off..font_off)
o.rmempty = true
o:value("过期时间")
o:value("剩余流量")
o:value("TG群")
o:value("官网")

local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(openclash, sid)
   luci.http.redirect(m.redirect)
end

return m
