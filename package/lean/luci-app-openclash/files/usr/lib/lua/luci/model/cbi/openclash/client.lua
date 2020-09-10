
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require("luci.model.uci").cursor()

m = Map("openclash")
m.title = translate("OpenClash")
m.description = translate("A Clash Client For OpenWrt")
m.pageaction = false

m:section(SimpleSection).template  = "openclash/status"
m:section(SimpleSection).template  = "openclash/state"

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

function config_check(CONFIG_FILE)
  local yaml = fs.isfile(CONFIG_FILE)
  local proxy,group,rule
  if yaml then
  	 proxy_provier = luci.sys.call(string.format('egrep "^ {0,}proxy-provider:" "%s" >/dev/null 2>&1',CONFIG_FILE))
  	 if (proxy_provier ~= 0) then
  	    proxy_provier = luci.sys.call(string.format('egrep "^ {0,}proxy-providers:" "%s" >/dev/null 2>&1',CONFIG_FILE))
  	 end
     proxy = luci.sys.call(string.format('egrep "^ {0,}Proxy:" "%s" >/dev/null 2>&1',CONFIG_FILE))
     if (proxy ~= 0) then
        proxy = luci.sys.call(string.format('egrep "^proxies:" "%s" >/dev/null 2>&1',CONFIG_FILE))
     end
     group = luci.sys.call(string.format('egrep " {0,}Proxy Group" "%s" >/dev/null 2>&1',CONFIG_FILE))
     if (group ~= 0) then
     	  group = luci.sys.call(string.format('egrep "^ {0,}proxy-groups:" "%s" >/dev/null 2>&1',CONFIG_FILE))
     end
     rule = luci.sys.call(string.format('egrep "^ {0,}Rule:" "%s" >/dev/null 2>&1',CONFIG_FILE))
     if (rule ~= 0) then
        rule = luci.sys.call(string.format('egrep "^ {0,}rules:" "%s" >/dev/null 2>&1',CONFIG_FILE))
        if (rule ~= 0) then
           rule = luci.sys.call(string.format('egrep "^ {0,}script:" "%s" >/dev/null 2>&1',CONFIG_FILE))
        end
     end
  end
  if yaml then
     if (proxy == 0) then
        proxy = ""
     else
        if (proxy_provier == 0) then
           proxy = ""
        else
           proxy = " - 代理服务器"
        end
     end
     if (group == 0) then
        group = ""
     else
        group = " - 策略组"
     end
     if (rule == 0) then
        rule = ""
     else
        rule = " - 规则"
     end
     if (proxy=="") and (group=="") and (rule=="") then
        return "Config Normal"
     else
	      return proxy..group..rule.." - 部分异常"
	   end
	elseif (yaml ~= 0) then
	   return "配置文件不存在"
	end
end

local e,a={}
for t,o in ipairs(fs.glob("/etc/openclash/config/*"))do
a=fs.stat(o)
if a then
e[t]={}
e[t].num=string.format(t)
e[t].name=fs.basename(o)
BACKUP_FILE="/etc/openclash/backup/".. e[t].name
CONFIG_FILE="/etc/openclash/config/".. e[t].name
if fs.mtime(BACKUP_FILE) then
   e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",fs.mtime(BACKUP_FILE))
else
   e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",a.mtime)
end
if string.sub(luci.sys.exec("uci get openclash.config.config_path 2>/dev/null"), 23, -2) == e[t].name then
   e[t].state=translate("Enable")
else
   e[t].state=translate("Disable")
end
e[t].check=translate(config_check(CONFIG_FILE))
end
end

form = Map("openclash")
form.pageaction = false
tb=form:section(Table,e)
st=tb:option(DummyValue,"state",translate("State"))
st.template="openclash/cfg_check"
nm=tb:option(DummyValue,"name",translate("Config Alias"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
ck=tb:option(DummyValue,"check",translate("启动参数检查"))
ck.template="openclash/cfg_check"

btnis=tb:option(Button,"switch",translate("Switch Config"))
btnis.template="openclash/other_button"
btnis.render=function(o,t,a)
if not e[t] then return false end
if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
a.display=""
else
a.display="none"
end
o.inputstyle="apply"
Button.render(o,t,a)
end
btnis.write=function(a,t)
fs.unlink("/tmp/Proxy_Group")
luci.sys.exec(string.format('uci set openclash.config.config_path="/etc/openclash/config/%s"',e[t].name))
uci:set("openclash", "config", "enable", 1)
uci:commit("openclash")
SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "client"))
end

s = Map("openclash")
s.pageaction = false
s:section(SimpleSection).template  = "openclash/myip"

local t = {
    {enable, disable}
}

ap = Map("openclash")
ap.pageaction = false

ss = ap:section(Table, t)

o = ss:option(Button, "enable") 
o.inputtitle = translate("Enable Clash")
o.inputstyle = "apply"
o.write = function()
  uci:set("openclash", "config", "enable", 1)
  uci:commit("openclash")
  SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
end

o = ss:option(Button, "disable")
o.inputtitle = translate("Disable Clash")
o.inputstyle = "reset"
o.write = function()
  uci:set("openclash", "config", "enable", 0)
  uci:commit("openclash")
  SYS.call("/etc/init.d/openclash stop >/dev/null 2>&1 &")
end

d = Map("openclash")
d.title = translate("Technical Support")
d.pageaction = false
d:section(SimpleSection).template  = "openclash/developer"

return m, form, s, ap, d
