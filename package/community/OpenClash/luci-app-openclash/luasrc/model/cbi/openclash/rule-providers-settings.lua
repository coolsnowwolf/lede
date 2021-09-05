
local m, s, o
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

m = Map(openclash,  translate("Rule Providers and Groups"))
m.pageaction = false
m.description=translate("Attention:")..
"<br/>"..translate("The game proxy is a test function and does not guarantee the availability of rules")..
"<br/>"..translate("Preparation steps:")..
"<br/>"..translate("1. In the <server and policy group management> page, create the policy group and node you are going to use, and apply the configuration (when adding nodes, you must select the policy group you want to join). Policy group type suggestion: fallback, game nodes must support UDP")..
"<br/>"..translate("2. Click the <manage third party game rules> or <manage third party rule set> button to enter the rule list and download the rules you want to use")..
"<br/>"..translate("3. On this page, set the corresponding configuration file and policy group of the rule you have downloaded, and save the settings")..
"<br/>"..
"<br/>"..translate("Use in normal mode (rules of the game only):")..
"<br/>"..translate("1. Select normal mode in <global settings> - <mode settings> - <running mode> and enable UDP traffic, then restart")..
"<br/>"..
"<br/>"..translate("Use in tun mode:")..
"<br/>"..translate("1. In the <global settings> - <version update tab>, download and install the corresponding mode core first")..
"<br/>"..translate("2. Select mixed mode, Tun mode or game mode in <global settings> - <mode settings> - <running mode> and restart")..
"<br/>"..
"<br/>"..translate("When setting this page, if the groups is empty, please go to the <server and group management> page to add")..
"<br/>"..
"<br/>"..translate("Introduction to rule set usage: https://lancellc.gitbook.io/clash/clash-config-file/rule-provider")

function IsRuleFile(e)
e=e or""
local e=string.lower(string.sub(e,-6,-1))
return e==".rules"
end

function IsYamlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-5,-1))
   return e == ".yaml"
end

function IsYmlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-4,-1))
   return e == ".yml"
end

-- [[ Edit Game Rule ]] --
s = m:section(TypedSection, "game_config", translate("Game Rules and Groups"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.rmempty = false

---- enable flag
o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

---- config
o = s:option(ListValue, "config", translate("Config File"))
o:value("all", translate("Use For All Config File"))
local e,a={}
for t,f in ipairs(fs.glob("/etc/openclash/config/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].name=fs.basename(f)
    if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
       o:value(e[t].name)
    end
  end
end

---- rule name
o = s:option(DynamicList, "rule_name", translate("Game Rule's Name"))
local e,a={}
for t,f in ipairs(fs.glob("/etc/openclash/game_rules/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].filename=fs.basename(f)
    if IsRuleFile(e[t].filename) then
       e[t].name=string.gsub(luci.sys.exec(string.format("grep ',%s$' /usr/share/openclash/res/game_rules.list |awk -F ',' '{print $1}' 2>/dev/null",e[t].filename)), "[\r\n]", "")
       if e[t].name ~= "" and e[t].name ~= nil then
          o:value(e[t].name)
       end
    end
  end
end

o.rmempty = true

---- Proxy Group
o = s:option(ListValue, "group", translate("Select Proxy Group"))
uci:foreach("openclash", "groups",
		function(s)
		  if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
o:value("DIRECT")
o:value("REJECT")
o.rmempty = true

-- [[ Edit Other Rule Provider ]] --
s = m:section(TypedSection, "rule_provider_config", translate("Other Rule Providers and Groups (Only TUN Core Support)"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.rmempty = false

---- enable flag
o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty     = false
o.default     = o.enabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "1"
end

---- config
o = s:option(ListValue, "config", translate("Config File"))
o:value("all", translate("Use For All Config File"))
local e,a={}
for t,f in ipairs(fs.glob("/etc/openclash/config/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].name=fs.basename(f)
    if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
       o:value(e[t].name)
    end
  end
end

---- rule name
o = s:option(DynamicList, "rule_name", translate("Rule Provider's Name"))
local e,a={}
for t,f in ipairs(fs.glob("/etc/openclash/rule_provider/*"))do
	a=fs.stat(f)
	if a then
    e[t]={}
    e[t].filename=fs.basename(f)
    if IsYamlFile(e[t].filename) or IsYmlFile(e[t].filename) then
       e[t].name=string.gsub(luci.sys.exec(string.format("grep ',%s$' /usr/share/openclash/res/rule_providers.list |awk -F ',' '{print $1}' 2>/dev/null",e[t].filename)), "[\r\n]", "")
       if e[t].name ~= "" and e[t].name ~= nil then
          o:value(e[t].name)
       end
    end
  end
end

o.rmempty = true

---- Proxy Group
o = s:option(ListValue, "group", translate("Select Proxy Group"))
uci:foreach("openclash", "groups",
		function(s)
		  if s.name ~= "" and s.name ~= nil then
			   o:value(s.name)
			end
		end)
o:value("DIRECT")
o:value("REJECT")
o.rmempty = true

o = s:option(Value, "interval", translate("Rule Providers Interval(s)"))
o.default = "86400"
o.rmempty = false

---- position
o = s:option(ListValue, "position", translate("Append Position"))
o.rmempty     = false
o:value("0", translate("Priority Match"))
o:value("1", translate("Extended Match"))

-- [[ Edit Custom Rule Provider ]] --
s = m:section(TypedSection, "rule_providers", translate("Custom Rule Providers and Groups (Only TUN Core Support)"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/openclash/rule-providers-config/%s")
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

o = s:option(DummyValue, "name", translate("Rule Providers Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(ListValue, "position", translate("Append Position"))
o.rmempty     = false
o:value("0", translate("Priority Match"))
o:value("1", translate("Extended Match"))

local rm = {
    {rule_mg, pro_mg}
}

rmg = m:section(Table, rm)

o = rmg:option(Button, "rule_mg", " ")
o.inputtitle = translate("Game Rules Manage")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "game-rules-manage"))
end

o = rmg:option(Button, "pro_mg", " ")
o.inputtitle = translate("Other Rule Provider Manage")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-manage"))
end

local t = {
    {Commit, Apply}
}

ss = m:section(Table, t)

o = ss:option(Button, "Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("openclash")
end

o = ss:option(Button, "Apply", " ")
o.inputtitle = translate("Apply Settings")
o.inputstyle = "apply"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

m:append(Template("openclash/toolbar_show"))

return m
