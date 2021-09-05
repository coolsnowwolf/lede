
local rule_form
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

btndl2 = tb2:option(Button,"download2",translate("Download Config"))
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

local t = {
    {Refresh, Delete_all, Apply}
}

a = rule_form:section(Table, t)

o = a:option(Button, "Refresh", " ")
o.inputtitle = translate("Refresh Page")
o.inputstyle = "apply"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-file-manage"))
end

o = a:option(Button, "Delete_all", " ")
o.inputtitle = translate("Delete All File")
o.inputstyle = "remove"
o.write = function()
  luci.sys.call("rm -rf /etc/openclash/rule_provider/* >/dev/null 2>&1")
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-file-manage"))
end

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "config"))
end

return rule_form
