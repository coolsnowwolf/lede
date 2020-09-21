
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"
local sys = require "luci.sys"
local sid = arg[1]

font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]


m = Map(openclash, translate("Config Subscribe Edit"))
m.pageaction = false
m.description=translate("Convert Subscribe function of Online is Supported By subconverter Written By tindy X") .. translate("<br/> \
<br/>API By tindy X & lhie1 \
<br/> \
<br/>subconverter 外部配置(订阅转换模板)说明：https://github.com/tindy2013/subconverter#external-configuration-file \
<br/> \
<br/>如需自定义外部配置文件(订阅转换模板)，请按照说明编写后上传至外部网络可访问的位置，并在使用时正确填写地址 \
<br/> \
<br/>如您有值得推荐的外部配置文件(订阅转换模板)，可以按照 /usr/share/openclash/res/sub_ini.list 的文件格式修改后提交PR")
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
o.description = font_red..bold_on..translate("(Name For Distinguishing)")..bold_off..font_off
o.placeholder = translate("config")
o.rmempty = true

---- address
o = s:option(Value, "address", translate("Subscribe Address"))
o.description = font_red..bold_on..translate("(Not Null)")..bold_off..font_off
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o.rmempty = false

---- subconverter
o = s:option(ListValue, "sub_convert", translate("Subscribe Convert Online"))
o.description = translate("Convert Subscribe Online With Template, Mix Proxies and Keep Settings options Will Not Effect")
o:value("0", translate("Disable"))
o:value("1", translate("Enable"))
o.default=0

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
o.description = font_red..bold_on..translate("(eg: hk or tw&bgp)")..bold_off..font_off
o.rmempty = true

---- exkey
o = s:option(DynamicList, "ex_keyword", font_red..bold_on..translate("Exclude Keyword Match")..bold_off..font_off)
o.description = font_red..bold_on..translate("(eg: hk or tw&bgp)")..bold_off..font_off
o.rmempty = true

local t = {
    {Commit, Back}
}
a = m:section(Table, t)

o = a:option(Button,"Commit")
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
   m.uci:commit(openclash)
   luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back")
o.inputtitle = translate("Back Configurations")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(openclash)
   luci.http.redirect(m.redirect)
end

return m
