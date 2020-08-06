
local m, s, o
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

m = Map(openclash,  translate("Config Update"))
m.pageaction = false

s = m:section(TypedSection, "openclash")
s.anonymous = true

---- update Settings
o = s:option(ListValue, "auto_update", translate("Auto Update"))
o.description = translate("Auto Update Server subscription")
o:value("0", translate("Disable"))
o:value("1", translate("Enable"))
o.default=0

o = s:option(ListValue, "config_auto_update_mode", translate("Update Mode"))
o:depends("auto_update", "1")
o:value("0", translate("Appointment Mode"))
o:value("1", translate("Loop Mode"))
o.default=0
o.rmempty = true

o = s:option(ListValue, "config_update_week_time", translate("Update Time (Every Week)"))
o:depends("config_auto_update_mode", "0")
o:value("*", translate("Every Day"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("0", translate("Every Sunday"))
o.default=1
o.rmempty = true

o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
o:depends("config_auto_update_mode", "0")
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.rmempty = true

o = s:option(Value, "config_update_interval", translate("Update Interval(min)"))
o.default="60"
o.datatype = "integer"
o:depends("config_auto_update_mode", "1")
o.rmempty = true

-- [[ Edit Server ]] --
s = m:section(TypedSection, "config_subscribe")
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.rmempty = false

---- enable flag
o = s:option(Flag, "enabled", translate("Enable"), font_red..bold_on..translate("(Enable or Disable Subscribe)")..bold_off..font_off)
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

---- name
o = s:option(Value, "name", translate("Config Alias"))
o.description = font_red..bold_on..translate("(Name For Distinguishing)")..bold_off..font_off
o.placeholder = translate("config")
o.rmempty = true

---- type
o = s:option(ListValue, "type", translate("Subscribe Type"))
o.description = font_red..bold_on..translate("(Power By fndroid)")..bold_off..font_off
o:value("clash", translate("Clash"))
o:value("v2rayn", translate("V2rayN"))
o:value("surge", translate("Surge"))
o.default="clash"
o.rempty = false

---- address
o = s:option(Value, "address", translate("Subscribe Address"))
o.description = font_red..bold_on..translate("(Not Null)")..bold_off..font_off
o.placeholder = translate("Not Null")
o.datatype = "or(host, string)"
o.rmempty = false

---- key
o = s:option(DynamicList, "keyword", font_red..bold_on..translate("Keyword Match")..bold_off..font_off)
o.description = font_red..bold_on..translate("(eg: hk or tw&bgp)")..bold_off..font_off
o.rmempty = true

---- exkey
o = s:option(DynamicList, "ex_keyword", font_red..bold_on..translate("Exclude Keyword Match")..bold_off..font_off)
o.description = font_red..bold_on..translate("(eg: hk or tw&bgp)")..bold_off..font_off
o.rmempty = true

local t = {
    {Commit, Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Commit") 
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  m.uci:commit("openclash")
end

o = a:option(Button, "Apply")
o.inputtitle = translate("Apply Configurations")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  uci:foreach("openclash", "config_subscribe",
		function(s)
		  if s.name ~= "" and s.name ~= nil and s.enabled == "1" then
			   local back_cfg_path_yaml="/etc/openclash/backup/" .. s.name .. ".yaml"
			   local back_cfg_path_yml="/etc/openclash/backup/" .. s.name .. ".yaml"
			   fs.unlink(back_cfg_path_yaml)
			   fs.unlink(back_cfg_path_yml)
			end
		end)
  SYS.call("/usr/share/openclash/openclash.sh >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

return m
