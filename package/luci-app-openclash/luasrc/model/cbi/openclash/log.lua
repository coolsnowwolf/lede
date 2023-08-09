--
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"

m = Map("openclash", translate("Server Logs"))
s = m:section(TypedSection, "openclash")
m.pageaction = false
s.anonymous = true
s.addremove=false

log = s:option(TextValue, "clog")
log.readonly=true
log.pollcheck=true
log.template="openclash/log"
log.description = translate("")
log.rows = 29

m:append(Template("openclash/toolbar_show"))
m:append(Template("openclash/config_editor"))

return m