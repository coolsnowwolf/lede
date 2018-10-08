local fs = require "nixio.fs"
local conffile = "/etc/shadowsocksr.json.backup" 

f = SimpleForm("general", translate("ShadowsocksR  - 备份服务器设置"), translate("ShadowsocksR 备份服务器设置地址，当主服务器不可时将自动连接到此服务器。  主服务器可用时将自动切换回主服务器"))

t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end

function f.handle(self, state, data)
	if state == FORM_VALID then
		if data.conf then
			fs.writefile(conffile, data.conf:gsub("\r\n", "\n"))
			luci.sys.call("/etc/init.d/shadowsocksr restart")
		end
	end
	return true
end

return f
