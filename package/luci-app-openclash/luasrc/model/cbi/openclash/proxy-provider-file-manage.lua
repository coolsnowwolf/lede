
local proxy_form
local openclash = "openclash"
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require "luci.model.uci".cursor()

local p,r={}
for x,y in ipairs(fs.glob("/etc/openclash/proxy_provider/*"))do
r=fs.stat(y)
if r then
p[x]={}
p[x].num=string.format(x)
p[x].name=fs.basename(y)
p[x].mtime=os.date("%Y-%m-%d %H:%M:%S",r.mtime)
p[x].size=fs.filesize(r.size)
p[x].remove=0
p[x].enable=false
end
end

proxy_form=SimpleForm("proxy_provider_file_list",translate("Proxy Provider File List"))
proxy_form.reset=false
proxy_form.submit=false
tb1=proxy_form:section(Table,p)
nu1=tb1:option(DummyValue,"num",translate("Serial Number"))
nm1=tb1:option(DummyValue,"name",translate("File Name"))
mt1=tb1:option(DummyValue,"mtime",translate("Update Time"))
sz1=tb1:option(DummyValue,"size",translate("Size"))

btned1=tb1:option(Button,"edit",translate("Edit"))
btned1.render=function(p,x,r)
p.inputstyle="apply"
Button.render(p,x,r)
end
btned1.write=function(r,x)
	local file_path = "etc/openclash/proxy_provider/" .. fs.basename(p[x].name)
	HTTP.redirect(DISP.build_url("admin", "services", "openclash", "other-file-edit", "proxy-provider-file-manage", "%s") %file_path)
end

btndl1 = tb1:option(Button,"download1",translate("Download Config"))
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
    {Refresh, Create, Delete_all, Apply}
}

a = proxy_form:section(Table, t)

o = a:option(Button, "Refresh", " ")
o.inputtitle = translate("Refresh Page")
o.inputstyle = "apply"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "proxy-provider-file-manage"))
end

o = a:option(DummyValue, "Create", " ")
o.rawhtml = true
o.template = "openclash/input_file_name"
o.value = "/etc/openclash/proxy_provider/"

o = a:option(Button, "Delete_all", " ")
o.inputtitle = translate("Delete All File")
o.inputstyle = "remove"
o.write = function()
  luci.sys.call("rm -rf /etc/openclash/proxy_provider/* >/dev/null 2>&1")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "proxy-provider-file-manage"))
end

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "config"))
end

proxy_form:append(Template("openclash/toolbar_show"))
return proxy_form
