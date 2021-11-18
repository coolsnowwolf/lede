mp = Map("openvpn")
mp.title = translate("OpenVPN Server")
mp.description = translate("An easy config OpenVPN Server Web-UI")

mp:section(SimpleSection).template = "openvpn-server/openvpn-server_status"

s = mp:section(TypedSection, "openvpn")
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enable"))

proto = s:option(Value, "proto", translate("Proto"))
proto:value("tcp4", translate("TCP Server IPv4"))
proto:value("udp4", translate("UDP Server IPv4"))
proto:value("tcp6", translate("TCP Server IPv6"))
proto:value("udp6", translate("UDP Server IPv6"))

port = s:option(Value, "port", translate("Port"))
port.datatype = "range(1,65535)"

ddns = s:option(Value, "ddns", translate("WAN DDNS or IP"))
ddns.datatype = "string"
ddns.default = "exmple.com"
ddns.rmempty = false

localnet = s:option(Value, "server", translate("Client Network"))
localnet.datatype = "string"
localnet.description = translate("VPN Client Network IP with subnet")

list = s:option(DynamicList, "push")
list.title = translate("Client Settings")
list.datatype = "string"
list.description = translate("Set route 192.168.0.0 255.255.255.0 and dhcp-option DNS 192.168.0.1 base on your router")

o = s:option(Button, "certificate", translate("OpenVPN Client config file"))
o.inputtitle = translate("Download .ovpn file")
o.description = translate("If you are using IOS client, please download this .ovpn file and send it via QQ or Email to your IOS device")
o.inputstyle = "reload"
o.write = function()
	luci.sys.call("sh /etc/genovpn.sh 2>&1 >/dev/null")
	Download()
end

function Download()
	local t,e
	t = nixio.open("/tmp/my.ovpn","r")
	luci.http.header('Content-Disposition','attachment; filename="my.ovpn"')
	luci.http.prepare_content("application/octet-stream")
	while true do
		e = t:read(nixio.const.buffersize)
		if (not e) or (#e==0) then
			break
		else
			luci.http.write(e)
		end
	end
	t:close()
	luci.http.close()
end

local pid = luci.util.exec("/usr/bin/pgrep openvpn")

function openvpn_process_status()
	local status = "OpenVPN is not running now "

	if pid ~= "" then
		status = "OpenVPN is running with the PID " .. pid .. ""
	end

	local status = { status = status }
	local table = { pid = status }
	return table
end

return mp
