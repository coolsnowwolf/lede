local fs = require "nixio.fs"
local conffile = "/tmp/ssrpro.log"

f = SimpleForm("logview")
f.reset = false
f.submit = false
t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
  luci.sys.exec("[ -f /tmp/ssrplus.log ] && sed '1!G;h;$!d' /tmp/ssrplus.log > /tmp/ssrpro.log")
	return fs.readfile(conffile) or ""
end
t.readonly="readonly"

return f
