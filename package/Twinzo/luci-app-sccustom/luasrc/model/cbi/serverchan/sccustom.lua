local m,s,a,c,f,x
local fs=require"nixio.fs"
local sys=require"luci.sys"
m=Map("sccustom",translate("自定义推送"),translate("农历二月三十需手动输入"))
-------------------------通用设置--------------------------------------
x = m:section(TypedSection, "global",translate("通用设置"),translate("可添加多个SCKEY").."调用代码获取<a href='http://sc.ftqq.com' target='_blank'>点击这里</a>")
x.optional = false
x.rmempty = false
x.addremove=true
x.anonymous=true
x.template="cbi/tblsection"

b = x:option(Flag, "enable", translate("Enable"))
b.optional = false
b.rmempty = false

b = x:option(Value, "sckey", translate("SCKEY"))

b = x:option(Value, "comment", translate("Comments"))
b.optional = false
b.rmempty = true

------------------------节假日-----------------------------
va = m:section(TypedSection, "vacation",translate("节假日"),translate("请仔细勾选农历，勾选则为农历，否则为新历"))
va.optional = false
va.rmempty = false
va.addremove=true
va.anonymous=true
va.template="cbi/tblsection"

b = va:option(Flag, "enable", translate("Enable"))
b.optional = false
b.rmempty = false

b = va:option(Datepicker, "date", translate("日期"))
b.optional = false
b.rmempty = true

b = va:option(Flag, "lunar", translate("农历"))
b.optional = false
b.rmempty = false

b = va:option(Timepicker, "time", translate("推送时间"))
b.optional = false
b.rmempty = true

title= va:option(Value, "send_title", translate("微信推送标题"))
title.placeholder = "国庆节"
title.optional = false
title.rmempty = true

b = va:option(TextValue, "messages", translate("推送信息"))
b.optional = false
b.rmempty = true
b.rows = 1
b.size = 20
b.wrap = "off"

b = va:option(Value, "comment", translate("Comments"))
b.optional = false
b.rmempty = true
---------------------------生日--------------------------
bir = m:section(TypedSection, "birthday",translate("生日"),translate("请仔细勾选农历，勾选则为农历，否则为新历"))
bir.optional = false
bir.rmempty = false
bir.addremove=true
bir.anonymous=true
bir.template="cbi/tblsection"

b = bir:option(Flag, "enable", translate("Enable"))
b.optional = false
b.rmempty = false

b = bir:option(Datepicker, "date", translate("日期"))
b.optional = false
b.rmempty = true

b = bir:option(Flag, "lunar", translate("农历"))
b.optional = false
b.rmempty = false

b = bir:option(Timepicker, "time", translate("推送时间"))
b.optional = false
b.rmempty = true

title= bir:option(Value, "send_title", translate("微信推送标题"))
title.placeholder = "xxx的生日"
title.optional = false
title.rmempty = true

b = bir:option(TextValue, "messages", translate("推送信息"))
b.optional = false
b.rmempty = true
b.rows = 1
b.size = 20
b.wrap = "off"

b = bir:option(Value, "comment", translate("Comments"))
b.optional = false
b.rmempty = true
-------------------------日常---------------------------
daliylife = m:section(TypedSection, "daliylife",translate("日常"),translate("0代表星期天，使用英文逗号“,”隔开。示例：0,1,2,3,4,5,6或不填为全周，1,2,3,4,5为周一到周五"))
daliylife.optional = false
daliylife.rmempty = false
daliylife.addremove=true
daliylife.anonymous=true
daliylife.template="cbi/tblsection"

b = daliylife:option(Flag, "enable", translate("Enable"))
b.optional = false
b.rmempty = false

dow = daliylife:option(Value, "daysofweek", translate("星期几"))
dow.optional = false
dow.rmempty = true
dow.placeholder="0,1,2,3,4,5,6"

b = daliylife:option(Timepicker, "time", translate("推送时间"))
b.optional = false
b.rmempty = true

title= daliylife:option(Value, "send_title", translate("微信推送标题"))
title.placeholder = "起床了"
title.optional = false
title.rmempty = true

b = daliylife:option(TextValue, "messages", translate("推送信息"))
b.optional = false
b.rmempty = true
b.rows = 1
b.size = 20
b.wrap = "off"

b = daliylife:option(Value, "comment", translate("Comments"))
b.optional = false
b.rmempty = true
-------------------log file-----------------------
f = m:section(TypedSection, "log", translate("日志"))
f.anonymous=true
local log_file="/tmp/serverchan/custom.log"
logfile=f:option(TextValue,"log_File")
logfile.description=translate("自定义消息推送日志")
logfile.rows=18
logfile.wrap="off"
logfile.readonly=true
function logfile.cfgvalue(s,s)
	sylogtext=""
	if log_file and nixio.fs.access(log_file) then
		log_File=luci.sys.exec("tail -n 100 %s"%log_file)
	end
return log_File
end
logfile.write=function(s,s,s)
end
---------------------datepicker-------------------------
c = m:section(TypedSection, "none")
c.anonymous=true
k=c:option(DummyValue,"none")
k.template="serverchan/datepicker"

return m