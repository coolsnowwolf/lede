--
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"


m = Map("openclash", translate("Server logs"))
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

a = SimpleForm("apply")
a.reset = false
a.submit = false
s = a:section(Table, t)

o = s:option(Button, "refresh") 
o.inputtitle = translate("Refresh Log")
o.inputstyle = "apply"
o.write = function()
  HTTP.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "log"))
end

o = s:option(Button, "clean")
o.inputtitle = translate("Clean Log")
o.inputstyle = "apply"
o.write = function()
  SYS.call("echo '' >/tmp/openclash.log")
end

return m, a