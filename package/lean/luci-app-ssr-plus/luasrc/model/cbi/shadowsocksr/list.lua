local fs = require "nixio.fs"
local conffile = "/etc/config/gfw.list"

f = SimpleForm("custom", translate("GFW Custom List"), translate("Please refer to the following writing"))

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
			luci.sys.call("/usr/share/shadowsocksr/gfw2ipset.sh && /etc/init.d/dnsmasq restart && ipset flush gfwlist")
		end
	end
	return true
end

return f