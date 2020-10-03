local fs = require "nixio.fs"
local conffile = "/usr/share/adbyby/adesc.conf"

f = SimpleForm("custom")

t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 13
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end

function f.handle(self, state, data)
	if state == FORM_VALID then
		if data.conf then
			fs.writefile(conffile, data.conf:gsub("\r\n", "\n"))
			luci.sys.call("/etc/init.d/adbyby restart")
		end
	end
	return true
end

return f

