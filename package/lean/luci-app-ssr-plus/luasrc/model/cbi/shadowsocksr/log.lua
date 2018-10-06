local fs = require "nixio.fs"
local conffile = "/tmp/ssrplus.log"

f = SimpleForm("custom")

t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end
t.readonly="readonly"

return f