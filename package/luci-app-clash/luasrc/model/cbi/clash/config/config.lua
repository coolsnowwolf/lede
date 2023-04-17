
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = luci.model.uci.cursor()
local fs = require "luci.clash"
local http = luci.http
local clash = "clash"


c = Map("clash")
c.template="clash/conf"


m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false

local conf = string.sub(luci.sys.exec("uci get clash.config.use_config"), 1, -2)
sev = s:option(TextValue, "conf")
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end

o=s:option(Button,"apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "reload"
o.write = function()
  m.uci:commit("clash")
end




local e,a={}
for t,o in ipairs(fs.glob("/usr/share/clash/config/sub/*.yaml"))do
a=fs.stat(o)
if a then
e[t]={}
e[t].name=fs.basename(o)
e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",a.mtime)
e[t].size=tostring(a.size)
e[t].remove1=0
e[t].enable=false
end
end

function IsYamlFile(e)
e=e or""
local e=string.lower(string.sub(e,-5,-1))
return e==".yaml"
end


f=Form("config_listst")
f.reset=false
f.submit=false
tb=f:section(Table,e, translate("Subcription Config List"))
nm=tb:option(DummyValue,"name",translate("File Name"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
sz=tb:option(DummyValue,"size",translate("Size"))

btnis=tb:option(Button,"switch1",translate("Use Config"))
btnis.template="clash/other_button"
btnis.render=function(o,t,a)
if not e[t]then return false end
if IsYamlFile(e[t].name)then
a.display=""
else
a.display="none"
end
o.inputstyle="apply"
Button.render(o,t,a)
end
btnis.write=function(a,t)
luci.sys.exec(string.format('uci set clash.config.use_config="/usr/share/clash/config/sub/%s"',e[t].name ))
luci.sys.exec('uci set clash.config.config_type="1"')
luci.sys.exec('uci commit clash')
if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
        luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
else
	HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config","config"))
end
end

btnist=tb:option(Button,"update1",translate("Update"))
btnist.template="clash/other_button"
btnist.render=function(o,t,a)
if not e[t]then return false end
o.inputstyle="apply"
Button.render(o,t,a)
end
btnist.write=function(a,t)
luci.sys.exec(string.format('uci set clash.config.config_update_name="%s"',e[t].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('bash /usr/share/clash/update.sh >>/usr/share/clash/clash.txt 2>&1 &')
luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
end



btndl = tb:option(Button,"download1",translate("Download")) 
btndl.template="clash/other_button"
btndl.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btndl.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/usr/share/clash/config/sub/"..e[t].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('yaml -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".yaml"
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


btnrm=tb:option(Button,"remove1",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
local a=fs.unlink("/usr/share/clash/config/sub/"..fs.basename(e[t].name))
luci.sys.exec(string.format('uci set clash.config.config_name_remove="%s"',e[t].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('bash /usr/share/clash/rmlist.sh 2>&1 &')
if a then table.remove(e,t)end
return a
end



local p,x={}
for q,v in ipairs(fs.glob("/usr/share/clash/config/upload/*.yaml"))do
x=fs.stat(v)
if x then
p[q]={}
p[q].name=fs.basename(v)
p[q].mtime=os.date("%Y-%m-%d %H:%M:%S",x.mtime)
p[q].size=tostring(x.size)
p[q].remove2=0
p[q].enable=false
end
end

fr=Form("config_list")
fr.reset=false
fr.submit=false
tb=fr:section(Table,p, translate("Upload Config List"))
nm=tb:option(DummyValue,"name",translate("File Name"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
sz=tb:option(DummyValue,"size",translate("Size"))

function IsYamlFile(p)
p=p or""
local p=string.lower(string.sub(p,-5,-1))
return p==".yaml"
end

btniss=tb:option(Button,"switch2",translate("Use Config"))
btniss.template="clash/other_button"
btniss.render=function(v,q,x)
if not p[q]then return false end
if IsYamlFile(p[q].name)then
x.display=""
else
x.display="none"
end
v.inputstyle="apply"
Button.render(v,q,x)
end
btniss.write=function(x,q)
luci.sys.exec(string.format('uci set clash.config.use_config="/usr/share/clash/config/upload/%s"',p[q].name ))
luci.sys.exec('uci set clash.config.config_type="2"')
luci.sys.exec('uci commit clash')
if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
        luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
else
	HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config" ,"config"))
end
end

btndll = tb:option(Button,"download2",translate("Download")) 
btndll.template="clash/other_button"
btndll.render=function(p,q,x)
p.inputstyle="remove"
Button.render(p,q,x)
end
btndll.write = function (x,q)
	local sPath, sFile, fd, block
	sPath = "/usr/share/clash/config/upload/"..p[q].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('yaml -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".yaml"
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


btnrml=tb:option(Button,"remove2",translate("Remove"))
btnrml.render=function(p,q,x)
p.inputstyle="remove"
Button.render(p,q,x)
end
btnrml.write=function(x,q)
local x=fs.unlink("/usr/share/clash/config/upload/"..fs.basename(p[q].name))
luci.sys.exec(string.format('uci set clash.config.config_up_remove="%s"',p[q].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('bash /usr/share/clash/uplist.sh 2>&1 &')
if x then table.remove(p,q)end
return x
end


local k,v={}
for c,z in ipairs(fs.glob("/usr/share/clash/config/custom/*.yaml"))do
v=fs.stat(z)
if v then
k[c]={}
k[c].name=fs.basename(z)
k[c].mtime=os.date("%Y-%m-%d %H:%M:%S",v.mtime)
k[c].size=tostring(v.size)
k[c].remove3=0
k[c].enable=false
end
end

fro=Form("config_listts")
fro.reset=false
fro.submit=false
tb=fro:section(Table,k,translate("Custom Config List"))
nm=tb:option(DummyValue,"name",translate("File Name"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
sz=tb:option(DummyValue,"size",translate("Size"))

function IsYamlFile(k)
k=k or""
local k=string.lower(string.sub(k,-5,-1))
return k==".yaml"
end

btnisz=tb:option(Button,"switch3",translate("Use Config"))
btnisz.template="clash/other_button"
btnisz.render=function(z,c,v)
if not k[c]then return false end
if IsYamlFile(k[c].name)then
v.display=""
else
v.display="none"
end
z.inputstyle="apply"
Button.render(z,c,v)
end
btnisz.write=function(v,c)
luci.sys.exec(string.format('uci set clash.config.use_config="/usr/share/clash/config/custom/%s"',k[c].name ))
luci.sys.exec('uci set clash.config.config_type="3"')
luci.sys.exec('uci commit clash')
if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
        luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
else
	HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "config"))
end
end




btndlz = tb:option(Button,"download3",translate("Download")) 
btndlz.template="clash/other_button"
btndlz.render=function(k,c,v)
k.inputstyle="remove"
Button.render(k,c,v)
end
btndlz.write = function (v,c)
	local sPath, sFile, fd, block
	sPath = "/usr/share/clash/config/custom/"..k[c].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('yaml -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".yaml"
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


btnrmz=tb:option(Button,"remove3",translate("Remove"))
btnrmz.render=function(k,c,v)
k.inputstyle="remove"
Button.render(k,c,v)
end
btnrmz.write=function(v,c)
local v=fs.unlink("/usr/share/clash/config/custom/"..fs.basename(k[c].name))
luci.sys.exec(string.format('uci set clash.config.config_cus_remove="%s"',k[c].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('/usr/share/clash/cuslist.sh 2>&1 &')

if v then table.remove(k,c)end
return v
end

return c,f,fr,fro,m