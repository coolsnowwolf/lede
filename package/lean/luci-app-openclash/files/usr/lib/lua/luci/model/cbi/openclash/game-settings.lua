
local m, s, o
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

m = Map(openclash,  translate("Game Rules and Groups"))
m.pageaction = false
m.description=translate("注意事项：<br/>游戏代理为测试功能，不保证可用性。 \
<br/>准备步骤：\
<br/>1、在《服务器与策略组管理》页面创建您准备使用的游戏策略组和游戏节点，并应用配置（节点添加时必须选择要加入的策略组），策略组类型建议:FallBack，游戏节点必须支持UDP \
<br/>2、点击《游戏规则管理》按钮进入游戏规则列表下载您要使用的游戏规则 \
<br/>3、在此页面上方设置您已下载的游戏规则的对应策略组并保存设置 \
<br/> \
<br/>在普通模式下使用： \
<br/>1、在《全局设置》-《模式设置》-《运行模式》中选择普通模式并启用UDP流量代理，然后重新启动。 \
<br/> \
<br/>在TUN模式下使用： \
<br/>1、替换内核一，下载地址：https://github.com/Dreamacro/clash/releases/tag/TUN \
<br/>2、或替换内核二，下载地址：https://github.com/vernesong/OpenClash/releases/tag/TUN \
<br/>3、项目地址：https://github.com/comzyh/clash \
<br/>4、在《全局设置》-《模式设置》-《运行模式》中选择TUN模式（内核一）或者游戏模式（内核二）并重新启动")


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
s = m:section(TypedSection, "game_config")
s.anonymous = true
s.addremove = true
s.sortable = false
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

local rm = {
    {rule_mg}
}

rmg = m:section(Table, rm)

o = rmg:option(Button, "rule_mg")
o.inputtitle = translate("Game Rules Manage")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "game-rules-manage"))
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
