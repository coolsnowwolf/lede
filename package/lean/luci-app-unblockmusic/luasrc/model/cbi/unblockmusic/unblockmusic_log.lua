local fs = require "nixio.fs"
local conffile = "/tmp/music.log"

f = SimpleForm("logview")

t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
  luci.sys.exec("grep -B 1 'http' /tmp/unblockmusic.log | grep -v -e'running' -e'TLS' -e'Transport' -e'POST' -e'github' -e'consumed' -e'starting' -e'error' > /tmp/music.log")
	return fs.readfile(conffile) or ""
end
t.readonly="readonly"

return f