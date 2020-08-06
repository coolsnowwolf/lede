
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
m.description=translate("注意事项：<br/>游戏代理为测试功能，不保证可用性 \
<br/>准备步骤：\
<br/>1、在《服务器与策略组管理》页面创建您准备使用的游戏策略组和游戏节点，并应用配置（节点添加时必须选择要加入的策略组），策略组类型建议:FallBack，游戏节点必须支持UDP \
<br/>2、点击《游戏规则管理》按钮进入游戏规则列表下载您要使用的游戏规则 \
<br/>3、在此页面上方设置您已下载的游戏规则的对应策略组并保存设置 \
<br/> \
<br/>在普通模式下使用： \
<br/>1、在《全局设置》-《模式设置》-《运行模式》中选择普通模式并启用UDP流量代理，然后重新启动 \
<br/> \
<br/>在TUN模式下使用： \
<br/>1、在全局设置-版本更新标签先下载对应模式内核 \
<br/>2、在《全局设置》-《模式设置》-《运行模式》中选择TUN模式或者游戏模式并重新启动 \
<br/> \
<br/>本页设置时如策略组为空，请先到《服务器与策略组管理》页面进行添加 \
<br/> \
<br/>规则集使用介绍：https://lancellc.gitbook.io/clash/clash-config-file/rule-provider")


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
       e[t].name=luci.sys.exec(string.format("grep -F '%s' /etc/openclash/game_rules.list |awk -F ',' '{print $1}' 2>/dev/null",e[t].filename))
       o:value(e[t].name)
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
s = m:section(TypedSection, "rule_provider_config", translate("Other Rule Providers and Groups"))
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
       e[t].name=luci.sys.exec(string.format("grep -F ',%s' /etc/openclash/rule_providers.list |awk -F ',' '{print $1}' 2>/dev/null",e[t].filename))
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
s = m:section(TypedSection, "rule_providers", translate("Custom Rule Providers and Groups"))
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

o = rmg:option(Button, "rule_mg")
o.inputtitle = translate("Game Rules Manage")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "game-rules-manage"))
end

o = rmg:option(Button, "pro_mg")
o.inputtitle = translate("Other Rule Provider Manage")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-manage"))
end

local t = {
    {Commit, Apply}
}

ss = m:section(Table, t)

o = ss:option(Button, "Commit") 
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("openclash")
end

o = ss:option(Button, "Apply")
o.inputtitle = translate("Apply Configurations")
o.inputstyle = "apply"
o.write = function()
  m.uci:set("openclash", "config", "enable", 1)
  m.uci:commit("openclash")
  SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

return m
