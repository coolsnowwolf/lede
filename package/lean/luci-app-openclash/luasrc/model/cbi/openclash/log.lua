--
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"

m = Map("openclash", translate("Server Logs"))
s = m:section(TypedSection, "openclash")
m.pageaction = false
s.anonymous = true
s.addremove=false

local clog = "/tmp/openclash.log"
log = s:option(TextValue, "clog")
log.readonly=true
log.description = translate("")
log.rows = 29
log.wrap = "off"
log.cfgvalue = function(self, section)
	return NXFS.readfile(clog) or ""
end
log.write = function(self, section, value)
end

local t = {
    {refresh, clean}
}

a = m:section(Table, t)

o = a:option(Button, "refresh") 
o.inputtitle = translate("Refresh Log")
o.inputstyle = "apply"
o.write = function()
  HTTP.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "log"))
end

o = a:option(Button, "clean")
o.inputtitle = translate("Clean Log")
o.inputstyle = "apply"
o.write = function()
  SYS.call("echo '' >/tmp/openclash.log")
end

return m