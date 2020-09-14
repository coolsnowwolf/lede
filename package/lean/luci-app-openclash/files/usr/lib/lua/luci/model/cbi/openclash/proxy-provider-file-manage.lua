
local proxy_form
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

local function i(e)
local t=0
local a={' KB',' MB',' GB',' TB'}
repeat
e=e/1024
t=t+1
until(e<=1024)
return string.format("%.1f",e)..a[t]
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

local t = {
    {Refresh, Delete_all, Apply}
}

a = proxy_form:section(Table, t)

o = a:option(Button, "Refresh")
o.inputtitle = translate("Refresh Page")
o.inputstyle = "apply"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "proxy-provider-file-manage"))
end

o = a:option(Button, "Delete_all")
o.inputtitle = translate("Delete All File")
o.inputstyle = "remove"
o.write = function()
  luci.sys.call("rm -rf /etc/openclash/proxy_provider/* >/dev/null 2>&1")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "proxy-provider-file-manage"))
end

o = a:option(Button, "Apply")
o.inputtitle = translate("Back Configurations")
o.inputstyle = "reset"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "config"))
end

return proxy_form
