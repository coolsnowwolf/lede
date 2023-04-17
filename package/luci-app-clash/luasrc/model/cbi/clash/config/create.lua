local clash = "clash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require "luci.model.uci".cursor()
local s, o, krk, z, r
local http = luci.http

font_blue = [[<font color="blue">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

krk = Map(clash)
s = krk:section(TypedSection, "clash", translate("Create Config"))
s.anonymous = true
--krk.pageaction = false

o = s:option(Flag, "provider_config", translate("Enable Create"))
o.default = 1
o.description = translate("Enable to create configuration")

o = s:option(Flag, "prox", translate("Use Proxy"))
o.description = translate("Use Proxy")


o = s:option(Flag, "ppro", translate("Use Proxy Provider"))
o.description = translate("Use Proxy Provider")

o = s:option(Flag, "rulprp", translate("Use Rule Provider"))
o.description = translate("Use Rule Provider")
o:depends("orul", 0)

o = s:option(Flag, "rul", translate("Use Rules"))
o.description = translate("Use Rules")
o:depends("orul", 0)

o = s:option(Flag, "orul", translate("Use Other Rules"))
o.description = translate("Use Other Rules")
o:depends("rul", 0)


o = s:option(Flag, "script", translate("Use Script"))
o.description = translate("Use Script")
o:depends("rul", 1)
o:depends("rulprp", 1)

o = s:option(Value, "name_tag")
o.title = translate("Config Name")
o.rmempty = true
o.description = translate("Give a name for your config")

cc = s:option(Flag, "same_tag", translate("Force Same Name"))
cc.default = 1
cc.description = translate("Enable to overwrite config file")

o = s:option(Button,"Manager")
o.title = translate("Rule Providers Manager")
o.inputtitle = translate("Rule Providers Manager")
o.inputstyle = "reload"
o.write = function()
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "rulemanager"))
end


local t = {
    {Creat_Config, Delete_Groups, Delete_ProxyPro, Delete_RulePro,Delete_Rules}
}

b = krk:section(Table, t)


o = b:option(Button,"Creat_Config")
o.inputtitle = translate("Create Config")
o.inputstyle = "apply"
o.write = function()
  krk.uci:commit("clash")
  luci.sys.call("bash /usr/share/clash/create/create.sh >/dev/null 2>&1 &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
end

o = b:option(Button,"Delete Severs")
o.inputtitle = translate("Delete Severs")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "servers", function(s) return true end)
  krk.uci:commit("clash")
end

o = b:option(Button,"Delete_ProxyPro")
o.inputtitle = translate("Delete Proxy Provider")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "proxyprovider", function(s) return true end)
  krk.uci:commit("clash")
end

o = b:option(Button,"Delete_RulePro")
o.inputtitle = translate("Delete Rule Provider")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "ruleprovider", function(s) return true end)
  krk.uci:commit("clash")
end


o = b:option(Button,"Delete_Rules")
o.inputtitle = translate("Delete Rules")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "rules", function(s) return true end)
  krk.uci:commit("clash")
end

o = b:option(Button,"Delete_Groups")
o.inputtitle = translate("Delete Groups")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "pgroups", function(s) return true end)
  krk.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "create"))
end


-- [[ Proxies ]]--
s = krk:section(TypedSection, "servers", translate("Proxies"))
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/clash/servers/%s")
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

o = s:option(DummyValue, "type", translate("Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "name", translate("Alias"))
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

o = s:option(DummyValue, "server" ,translate("Latency"))
o.template="clash/ping"
o.width="10%"


-- [[ Groups Manage ]]--
x = krk:section(TypedSection, "pgroups", translate("Policy Groups"))
x.description = font_blue..bold_on..translate(" https://lancellc.gitbook.io/clash/clash-config-file/proxy-groups")..bold_off..font_off..' '
x.anonymous = true
x.addremove = true
x.sortable = true
x.template = "cbi/tblsection"
x.extedit = luci.dispatcher.build_url("admin/services/clash/pgroups/%s")
function x.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(x.extedit % sid)
		return
	end
end

o = x:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = x:option(DummyValue, "type", translate("Group Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


o = x:option(DummyValue, "name", translate("Group Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


-- [[ Proxy-Provider Manage ]]--
z = krk:section(TypedSection, "proxyprovider", translate("Proxy Provider"))
z.description = font_blue..bold_on..translate(" https://lancellc.gitbook.io/clash/clash-config-file/proxy-provider")..bold_off..font_off..' '
z.anonymous = true
z.addremove = true
z.sortable = true
z.template = "cbi/tblsection"
z.extedit = luci.dispatcher.build_url("admin/services/clash/proxyprovider/%s")
function z.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(z.extedit % sid)
		return
	end
end

o = z:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = z:option(DummyValue, "name", translate("Provider Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = z:option(DummyValue, "type", translate("Provider Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end



-- [[ Rule-Provider Manage ]]--
r = krk:section(TypedSection, "ruleprovider", translate("Rule Provider"))
r.description = font_blue..bold_on..translate(" https://lancellc.gitbook.io/clash/clash-config-file/rule-provider")..bold_off..font_off..' '
r.anonymous = true
r.addremove = true
r.sortable = true
r.template = "cbi/tblsection"
r.extedit = luci.dispatcher.build_url("admin/services/clash/ruleprovider/%s")
function r.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(r.extedit % sid)
		return
	end
end

o = r:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end


o = r:option(DummyValue, "name", translate("Provider Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = r:option(DummyValue, "type", translate("Provider Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = r:option(DummyValue, "behavior", translate("Provider Behavior"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


-- [[ Rule Manage ]]--
q = krk:section(TypedSection, "rules", translate("Rules"))
q.description = font_blue..bold_on..translate(" https://lancellc.gitbook.io/clash/clash-config-file/rules")..bold_off..font_off..' '
q.anonymous = true
q.addremove = true
q.sortable = true
q.template = "cbi/tblsection"
q.extedit = luci.dispatcher.build_url("admin/services/clash/rules/%s")
function q.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(q.extedit % sid)
		return
	end
end

o = q:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

o = q:option(DummyValue, "type", translate("Rule Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = q:option(DummyValue, "rulename", translate("Description"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("-")
end


o = q:option(DummyValue, "rulegroups", translate("Groups"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


m = Map("clash")
y = m:section(TypedSection, "clash" , translate("Script"))
y.anonymous = true
y.addremove=false


local script="/usr/share/clash/create/script.yaml"
sev = y:option(TextValue, "scriptt")
sev.description =translate("NB: Set Clash Mode to Script if want to use")..font_blue..bold_on..translate(" https://lancellc.gitbook.io/clash/clash-config-file/script")..bold_off..font_off..' '
sev.rows = 10
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(script) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(script, value:gsub("\r\n", "\n"))
end


l = Map("clash")
v = l:section(TypedSection, "clash" , translate("Other Rules"))
v.anonymous = true
v.addremove=false


o = v:option(Value, "rule_url")
o.title = translate("Custom Rule Url")
o.description = translate("Insert your custom rule Url and click download")
o.rmempty = true

o = v:option(Button,"rule_update")
o.title = translate("Download Rule")
o.inputtitle = translate("Download Rule")
o.description = translate("Download Rule")
o.inputstyle = "reload"
o.write = function()
  uci:commit("clash")
  luci.sys.call("bash /usr/share/clash/rule.sh >>/usr/share/clash/clash.txt >/dev/null 2>&1 &")
end

local rule = "/usr/share/clash/rule.yaml"
sev = v:option(TextValue, "rule")
sev.description = translate("NB: Attention to Proxy Group and Rule when making changes to this section")
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(rule) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(rule, value:gsub("\r\n", "\n"))
end

o = v:option(Button,"del_rule")
o.inputtitle = translate("Delete Rule")
o.write = function()
  SYS.call("rm -rf /usr/share/clash/rule.yaml >/dev/null 2>&1 &")
end

krk:append(Template("clash/list"))

return krk,m,l

