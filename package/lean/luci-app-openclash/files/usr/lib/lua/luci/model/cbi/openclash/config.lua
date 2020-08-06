
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require("luci.model.uci").cursor()
local CHIF = "0"

font_green = [[<font color="green">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

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
    
ful = SimpleForm("upload", translate("Config Manage"), nil)
ful.reset = false
ful.submit = false

sul =ful:section(SimpleSection, "")
o = sul:option(FileUpload, "")
o.template = "openclash/upload"
um = sul:option(DummyValue, "", nil)
um.template = "openclash/dvalue"

local dir, fd, clash
clash = "/etc/openclash/clash"
dir = "/etc/openclash/config/"
bakck_dir="/etc/openclash/backup"
proxy_pro_dir="/etc/openclash/proxy_provider/"
rule_pro_dir="/etc/openclash/rule_provider/"
create_bakck_dir=fs.mkdir(bakck_dir)
create_proxy_pro_dir=fs.mkdir(proxy_pro_dir)
create_rule_pro_dir=fs.mkdir(rule_pro_dir)


HTTP.setfilehandler(
	function(meta, chunk, eof)
		local fp = HTTP.formvalue("file_type")
		if not fd then
			if not meta then return end
			
			if fp == "config" then
			   if meta and chunk then fd = nixio.open(dir .. meta.file, "w") end
			elseif fp == "proxy-provider" then
			   if meta and chunk then fd = nixio.open(proxy_pro_dir .. meta.file, "w") end
			elseif fp == "rule-provider" then
			   if meta and chunk then fd = nixio.open(rule_pro_dir .. meta.file, "w") end
			end

			if not fd then
				um.value = translate("upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
      if IsYamlFile(meta.file) and fp == "config" then
         local yamlbackup="/etc/openclash/backup/" .. meta.file
         local c=fs.copy(dir .. meta.file,yamlbackup)
      end
      if IsYmlFile(meta.file) and fp == "config" then
      	 local ymlname=string.lower(string.sub(meta.file,0,-5))
         local ymlbackup="/etc/openclash/backup/".. ymlname .. ".yaml"
         local c=fs.rename(dir .. meta.file,"/etc/openclash/config/".. ymlname .. ".yaml")
         local c=fs.copy("/etc/openclash/config/".. ymlname .. ".yaml",ymlbackup)
      end
			if fp == "config" then
			   um.value = translate("File saved to") .. ' "/etc/openclash/config/"'
			   CHIF = "1"
			elseif fp == "proxy-provider" then
				 um.value = translate("File saved to") .. ' "/etc/openclash/proxy_provider/"'
			elseif fp == "rule-provider" then
				 um.value = translate("File saved to") .. ' "/etc/openclash/rule_provider/"'
			end
			fs.unlink("/tmp/Proxy_Group")
		end
	end
)

if HTTP.formvalue("upload") then
	local f = HTTP.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end

local function i(e)
local t=0
local a={' KB',' MB',' GB',' TB'}
repeat
e=e/1024
t=t+1
until(e<=1024)
return string.format("%.1f",e)..a[t]
end

local e,a={}
for t,o in ipairs(fs.glob("/etc/openclash/config/*"))do
a=fs.stat(o)
if a then
e[t]={}
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
e[t].size=i(a.size)
e[t].check=translate(config_check(CONFIG_FILE))
e[t].remove=0
end
end
    
form=SimpleForm("config_file_list",translate("Config File List"))
form.reset=false
form.submit=false
tb=form:section(Table,e)
st=tb:option(DummyValue,"state",translate("State"))
st.template="openclash/cfg_check"
nm=tb:option(DummyValue,"name",translate("Config Alias"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
sz=tb:option(DummyValue,"size",translate("Size"))
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
uci:commit("openclash")
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "config"))
end

btndl = tb:option(Button,"download",translate("Download Configurations")) 
btndl.template="openclash/other_button"
btndl.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btndl.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/etc/openclash/config/"..e[t].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		return
	end
	HTTP.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	HTTP.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			HTTP.write(block)
		end
	end
	fd:close()
	HTTP.close()
end

btnrm=tb:option(Button,"remove",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="reset"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
	fs.unlink("/tmp/Proxy_Group")
	fs.unlink("/etc/openclash/backup/"..luci.openclash.basename(e[t].name))
	fs.unlink("/etc/openclash/history/"..luci.openclash.basename(e[t].name))
	local a=fs.unlink("/etc/openclash/config/"..luci.openclash.basename(e[t].name))
if a then table.remove(e,t)end
return a
end

local p,r={}
for x,y in ipairs(fs.glob("/etc/openclash/proxy_provider/*"))do
r=fs.stat(y)
if r then
p[x]={}
p[x].name=fs.basename(y)
p[x].mtime=os.date("%Y-%m-%d %H:%M:%S",r.mtime)
p[x].size=i(r.size)
p[x].remove=0
p[x].enable=false
end
end

proxy_form=SimpleForm("proxy_provider_file_list",translate("Proxy Provider File List"))
proxy_form.reset=false
proxy_form.submit=false
tb1=proxy_form:section(Table,p)
nm1=tb1:option(DummyValue,"name",translate("File Name"))
mt1=tb1:option(DummyValue,"mtime",translate("Update Time"))
sz1=tb1:option(DummyValue,"size",translate("Size"))

btndl1 = tb1:option(Button,"download1",translate("Download Configurations")) 
btndl1.template="openclash/other_button"
btndl1.render=function(y,x,r)
y.inputstyle="remove"
Button.render(y,x,r)
end
btndl1.write = function (r,x)
	local sPath, sFile, fd, block
	sPath = "/etc/openclash/proxy_provider/"..p[x].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		return
	end
	HTTP.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	HTTP.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			HTTP.write(block)
		end
	end
	fd:close()
	HTTP.close()
end

btnrm1=tb1:option(Button,"remove1",translate("Remove"))
btnrm1.render=function(p,x,r)
p.inputstyle="reset"
Button.render(p,x,r)
end
btnrm1.write=function(r,x)
local r=fs.unlink("/etc/openclash/proxy_provider/"..luci.openclash.basename(p[x].name))
if r then table.remove(p,x)end
return r
end

local g,h={}
for n,m in ipairs(fs.glob("/etc/openclash/rule_provider/*"))do
h=fs.stat(m)
if h then
g[n]={}
g[n].name=fs.basename(m)
g[n].mtime=os.date("%Y-%m-%d %H:%M:%S",h.mtime)
g[n].size=i(h.size)
g[n].remove=0
g[n].enable=false
end
end

rule_form=SimpleForm("rule_provider_file_list",translate("Rule Providers File List"))
rule_form.reset=false
rule_form.submit=false
tb2=rule_form:section(Table,g)
nm2=tb2:option(DummyValue,"name",translate("File Name"))
mt2=tb2:option(DummyValue,"mtime",translate("Update Time"))
sz2=tb2:option(DummyValue,"size",translate("Size"))

btndl2 = tb2:option(Button,"download2",translate("Download Configurations")) 
btndl2.template="openclash/other_button"
btndl2.render=function(m,n,h)
m.inputstyle="remove"
Button.render(m,n,h)
end
btndl2.write = function (h,n)
	local sPath, sFile, fd, block
	sPath = "/etc/openclash/rule_provider/"..g[n].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		return
	end
	HTTP.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	HTTP.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			HTTP.write(block)
		end
	end
	fd:close()
	HTTP.close()
end

btnrm2=tb2:option(Button,"remove2",translate("Remove"))
btnrm2.render=function(g,n,h)
g.inputstyle="reset"
Button.render(g,n,h)
end
btnrm2.write=function(h,n)
local h=fs.unlink("/etc/openclash/rule_provider/"..luci.openclash.basename(g[n].name))
if h then table.remove(g,n)end
return h
end

m = SimpleForm("openclash")
m.reset = false
m.submit = false

local tab = {
 {user, default}
}

s = m:section(Table, tab)

local conf = string.sub(luci.sys.exec("uci get openclash.config.config_path 2>/dev/null"), 1, -2)
local dconf = "/etc/openclash/default.yaml"
local conf_name = fs.basename(conf)
if not conf_name or conf == "" then conf_name = "config.yaml" end

sev = s:option(Value, "user")
sev.template = "cbi/tvalue"
sev.description = translate("Modify Your Config file:").." "..font_green..bold_on..conf_name..bold_off..font_off.." "..translate("Here, Except The Settings That Were Taken Over")
sev.rows = 40
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or NXFS.readfile(dconf) or ""
end
sev.write = function(self, section, value)
if (CHIF == "0") then
    value = value:gsub("\r\n?", "\n")
    NXFS.writefile(conf, value)
end
end

def = s:option(Value, "default")
def.template = "cbi/tvalue"
def.description = translate("Default Config File With Correct General-Settings")
def.rows = 40
def.wrap = "off"
def.readonly = true
def.cfgvalue = function(self, section)
	return NXFS.readfile(dconf) or ""
end
def.write = function(self, section, value)
end


local t = {
    {Commit, Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Commit") 
o.inputtitle = translate("Commit Configurations")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  uci:commit("openclash")
end

o = a:option(Button, "Apply")
o.inputtitle = translate("Apply Configurations")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
  uci:set("openclash", "config", "enable", 1)
  uci:commit("openclash")
  SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

return ful , form , proxy_form , rule_form , m
