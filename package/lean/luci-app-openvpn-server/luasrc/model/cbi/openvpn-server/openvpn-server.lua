
--require("luci.tools.webadmin")

mp = Map("openvpn", "OpenVPN Server",translate("An easy config OpenVPN Server Web-UI"))

mp:section(SimpleSection).template  = "openvpn/openvpn_status"

s = mp:section(TypedSection, "openvpn")
s.anonymous = true
s.addremove = false

s:tab("basic",  translate("Base Setting"))

o = s:taboption("basic", Flag, "enabled", translate("Enable"))

proto = s:taboption("basic",Value,"proto", translate("Proto"))
proto:value("tcp", translate("TCP Server"))
proto:value("udp", translate("UDP Server"))

port = s:taboption("basic", Value, "port", translate("Port"))
port.datatype = "range(1,65535)"

ddns = s:taboption("basic", Value, "ddns", translate("WAN DDNS or IP"))
ddns.datatype = "string"
ddns.default = "exmple.com"
ddns.rmempty = false

localnet = s:taboption("basic", Value, "server", translate("Client Network"))
localnet.datatype = "string"
localnet.description = translate("VPN Client Network IP with subnet")

list = s:taboption("basic", DynamicList, "push")
list.title = translate("Client Settings")
list.datatype = "string"
list.description = translate("Set route 192.168.0.0 255.255.255.0 and dhcp-option DNS 192.168.0.1 base on your router")


local o
o = s:taboption("basic", Button,"certificate",translate("OpenVPN Client config file"))
o.inputtitle = translate("Download .ovpn file")
o.description = translate("If you are using IOS client, please download this .ovpn file and send it via QQ or Email to your IOS device")
o.inputstyle = "reload"
o.write = function()
  luci.sys.call("sh /etc/genovpn.sh 2>&1 >/dev/null")
	Download()
end

s:tab("code",  translate("Special Code"))

local conf = "/etc/ovpnadd.conf"
local NXFS = require "nixio.fs"
o = s:taboption("code", TextValue, "conf")
o.description = translate("(!)Special Code you know that add in to client .ovpn file")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end


local pid = luci.util.exec("/usr/bin/pgrep openvpn")

function openvpn_process_status()
  local status = "OpenVPN is not running now "

  if pid ~= "" then
      status = "OpenVPN is running with the PID " .. pid .. ""
  end

  local status = { status=status }
  local table = { pid=status }
  return table
end



function Download()
	local t,e
	t=nixio.open("/tmp/my.ovpn","r")
	luci.http.header('Content-Disposition','attachment; filename="my.ovpn"')
	luci.http.prepare_content("application/octet-stream")
	while true do
		e=t:read(nixio.const.buffersize)
		if(not e)or(#e==0)then
			break
		else
			luci.http.write(e)
		end
	end
	t:close()
	luci.http.close()
end

function mp.on_after_commit(self)
  os.execute("uci set firewall.openvpn.dest_port=$(uci get openvpn.myvpn.port) && uci commit firewall &&  /etc/init.d/firewall restart")
  os.execute("/etc/init.d/openvpn restart")
end


return mp
