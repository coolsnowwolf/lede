require "luci.util"
require "nixio.fs"
f = SimpleForm("logview")
f.reset = false
f.submit = false
t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
	if nixio.fs.access("/var/log/ssrplus.log") then
		local logs = luci.util.execi("cat /var/log/ssrplus.log")
		local s = ""
		for line in logs do
			s = line .. "\n" .. s
		end
		return s
	end
end
t.readonly = "readonly"
return f
