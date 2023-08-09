
local m, s, o
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

font_red = [[<b style=color:red>]]
font_off = [[</b>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

m = Map("openclash",  translate("Config Subscribe"))
m.pageaction = false

s = m:section(TypedSection, "openclash")
s.anonymous = true

---- update Settings
o = s:option(Flag, "auto_update", translate("Auto Update"))
o.description = translate("Auto Update Server subscription")
o.default = 0

o = s:option(ListValue, "config_auto_update_mode", translate("Update Mode"))
o:depends("auto_update", "1")
o:value("0", translate("Appointment Mode"))
o:value("1", translate("Loop Mode"))
o.default = "0"
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
o.default = "1"
o.rmempty = true

o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
o:depends("config_auto_update_mode", "0")
for t = 0,23 do
o:value(t, t..":00")
end
o.default = "0"
o.rmempty = true

o = s:option(Value, "config_update_interval", translate("Update Interval(min)"))
o.default = "60"
o.datatype = "integer"
o:depends("config_auto_update_mode", "1")
o.rmempty = true

-- [[ Edit Server ]] --
s = m:section(TypedSection, "config_subscribe", translate("Config Subscribe Edit"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/openclash/config-subscribe-edit/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

---- enable flag
o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

---- name
o = s:option(DummyValue, "name", translate("Config Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("config")
end

---- address
o = s:option(DummyValue, "address", translate("Subscribe Address"))
function o.cfgvalue(...)
	if Value.cfgvalue(...) then
		if string.len(Value.cfgvalue(...)) <= 50 then
			return Value.cfgvalue(...)
		else
			return string.sub(Value.cfgvalue(...), 1, 50) .. " ..."
		end
	end
	return translate("None")
end

---- template
o = s:option(DummyValue, "template", translate("Template Name"))
function o.cfgvalue(...)
	if Value.cfgvalue(...) ~= "0" then
		return Value.cfgvalue(...) or translate("None")
	else
		return translate("Custom Template")
	end
end

local t = {
    {Commit, Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  m.uci:commit("openclash")
end

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Update Config")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  uci:foreach("openclash", "config_subscribe",
		function(s)
		  if s.name ~= "" and s.name ~= nil and s.enabled == "1" then
			   local back_cfg_path_yaml="/etc/openclash/backup/" .. s.name .. ".yaml"
			   local back_cfg_path_yml="/etc/openclash/backup/" .. s.name .. ".yml"
			   fs.unlink(back_cfg_path_yaml)
			   fs.unlink(back_cfg_path_yml)
			end
		end)
  SYS.call("/usr/share/openclash/openclash.sh >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

m:append(Template("openclash/toolbar_show"))

return m
