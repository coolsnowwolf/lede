
local m, s, o
local openclash = "openclash"
local uci = luci.model.uci.cursor()
local fs = require "luci.openclash"

font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

m = Map(openclash,  translate("Servers manage and Config create"))
m.pageaction = false

s = m:section(TypedSection, "openclash")
s.anonymous = true

o = s:option(ListValue, "create_config", translate("Create Config"))
o.description = font_red .. bold_on .. translate("Create Config By One-Click Only Need Proxys") .. bold_off .. font_off
o:value("0", translate("Disable"))
o:value("1", translate("Enable"))
o.default=0

o = s:option(ListValue, "rule_sources", translate("Choose Template For Create Config"))
o.description = translate("Use Other Rules To Create Config")
o:depends("create_config", 1)
o:value("lhie1", translate("lhie1 Rules"))
o:value("ConnersHua", translate("ConnersHua Rules"))
o:value("ConnersHua_return", translate("ConnersHua Return Rules"))

o = s:option(ListValue, "servers_update", translate("Keep Settings"))
o.description = font_red .. bold_on .. translate("Only Update Servers Below When Subscription") .. bold_off .. font_off
o:value("0", translate("Disable"))
o:value("1", translate("Enable"))
o.default=0

o = s:option(DynamicList, "new_servers_group", translate("New Servers Group"))
o.description = translate("Set The New Subscribe Server's Default Proxy Groups")
o.rmempty = true
o:depends("servers_update", 1)
m.uci:foreach("openclash", "groups",
		function(s)
			o:value(s.name)
		end)

-- [[ Groups Manage ]]--
s = m:section(TypedSection, "groups", translate("Proxy Groups(No Need Set when Config Create)"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/openclash/groups-config/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(DummyValue, "config", translate("Config File"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("all")
end

o = s:option(DummyValue, "type", translate("Group Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "name", translate("Group Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

-- [[ Proxy-Provider Manage ]]--
s = m:section(TypedSection, "proxy-provider", translate("Proxy-Provider"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/openclash/proxy-provider-config/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = s:option(DummyValue, "config", translate("Config File"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("all")
end

o = s:option(DummyValue, "type", translate("Provider Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "name", translate("Provider Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

-- [[ Servers Manage ]]--
s = m:section(TypedSection, "servers", translate("Proxys"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/openclash/servers-config/%s")
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

o = s:option(DummyValue, "config", translate("Config File"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("all")
end

o = s:option(DummyValue, "type", translate("Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "name", translate("Server Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "server", translate("Server Address"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "port", translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue,"server",translate("Ping Latency"))
o.template="openclash/ping"
o.width="10%"

local tt = {
    {Delete_Unused_Servers, Delete_Severs, Delete_Proxy_Provider, Delete_Groups}
}

b = m:section(Table, tt)

o = b:option(Button,"Delete_Unused_Servers")
o.inputtitle = translate("Delete Unused Servers")
o.inputstyle = "reset"
o.write = function()
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:commit("openclash")
  luci.sys.call("sh /usr/share/openclash/cfg_unused_servers_del.sh 2>/dev/null")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "servers"))
end

o = b:option(Button,"Delete_Severs")
o.inputtitle = translate("Delete Severs")
o.inputstyle = "reset"
o.write = function()
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:delete_all("openclash", "servers", function(s) return true end)
  m.uci:commit("openclash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "servers"))
end

o = b:option(Button,"Delete_Proxy_Provider")
o.inputtitle = translate("Delete Proxy Provider")
o.inputstyle = "reset"
o.write = function()
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:delete_all("openclash", "proxy-provider", function(s) return true end)
  m.uci:commit("openclash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "servers"))
end

o = b:option(Button,"Delete_Groups")
o.inputtitle = translate("Delete Groups")
o.inputstyle = "reset"
o.write = function()
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:delete_all("openclash", "groups", function(s) return true end)
  m.uci:commit("openclash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "servers"))
end

local t = {
    {Load_Config, Commit, Apply}
}

a = m:section(Table, t)

o = a:option(Button,"Load_Config")
o.inputtitle = translate("Load Config")
o.inputstyle = "apply"
o.write = function()
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:commit("openclash")
  luci.sys.call("/usr/share/openclash/yml_groups_get.sh 2>/dev/null &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "openclash"))
end

o = a:option(Button, "Commit") 
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:commit("openclash")
end

o = a:option(Button, "Apply")
o.inputtitle = translate("Apply Configurations")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  m.uci:set("openclash", "config", "enable", 0)
  m.uci:commit("openclash")
  luci.sys.call("/usr/share/openclash/yml_groups_set.sh >/dev/null 2>&1 &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "openclash"))
end

m:append(Template("openclash/server_list"))
return m
