mp = Map("openvpn")

s = mp:section(TypedSection, "openvpn")
s.anonymous = true
s.addremove = false

local conf = "/etc/ovpnadd.conf"
local NXFS = require "nixio.fs"

o = s:option(TextValue, "conf")
o.description = translate("(!)Special Code you know that add in to client .ovpn file")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end

function mp.on_after_commit(self)
	os.execute("uci set firewall.openvpn.dest_port=$(uci get openvpn.myvpn.port) && uci commit firewall &&  /etc/init.d/firewall restart")
	os.execute("/etc/openvpncert.sh > /dev/null")
	os.execute("/etc/init.d/openvpn restart")
end

return mp
