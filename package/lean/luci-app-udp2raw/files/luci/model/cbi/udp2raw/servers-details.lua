local m, s, o
local sid = arg[1]

local raw_modes = {
	"faketcp",
	"udp",
	"icmp",
}

local cipher_modes = {
	"aes128cbc",
	"xor",
	"none",
}

local auth_modes = {
	"md5",
	"crc32",
	"simple",
	"none",
}

m = Map("udp2raw", "%s - %s" %{translate("udp2raw-tunnel"), translate("Edit Server")})
m.redirect = luci.dispatcher.build_url("admin/services/udp2raw/servers")
m.sid = sid

if m.uci:get("udp2raw", sid) ~= "servers" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove = false

o = s:option(Value, "alias", translate("Alias(optional)"))

o = s:option(Value, "server_addr", translate("Server"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.placeholder = "8080"

o = s:option(Value, "listen_addr", translate("Local Listen Host"))
o.datatype = "ipaddr"
o.placeholder = "127.0.0.1"

o = s:option(Value, "listen_port", translate("Local Listen Port"))
o.datatype = "port"
o.placeholder = "2080"

o = s:option(ListValue, "raw_mode", translate("Raw Mode"))
for _, v in ipairs(raw_modes) do o:value(v, v:lower()) end
o.default = "faketcp"
o.rmempty = false

o = s:option(Value, "key", translate("Password"))
o.password = true

o = s:option(ListValue, "cipher_mode", translate("Cipher Mode"))
for _, v in ipairs(cipher_modes) do o:value(v, v:lower()) end
o.default = "aes128cbc"

o = s:option(ListValue, "auth_mode", translate("Auth Mode"))
for _, v in ipairs(auth_modes) do o:value(v, v:lower()) end
o.default = "md5"

o = s:option(Flag, "auto_rule", translate("Auto Rule"), translate("Auto add (and delete) iptables rule."))
o.default = "1"

o = s:option(Flag, "keep_rule", translate("Keep Rule"), translate("Monitor iptables and auto re-add if necessary."))
o:depends("auto_rule", "1")

o = s:option(Value, "seq_mode", translate("seq Mode"), translate("seq increase mode for faketcp."))
o.datatype = "range(0,4)"
o.placeholder = "3"

o = s:option(Value, "lower_level", translate("Lower Level"), translate("Send packets at OSI level 2, format: \"eth0#00:11:22:33:44:55\", or \"auto\"."))

o = s:option(Value, "source_ip", translate("Source-IP"), translate("Force source-ip for Raw Socket."))
o.datatype = "ipaddr"

o = s:option(Value, "source_port", translate("Source-Port"), translate("Force source-port for Raw Socket, TCP/UDP only."))
o.datatype = "port"

o = s:option(Value, "log_level", translate("Log Level"))
o.datatype = "range(0,6)"
o.placeholder = "4"

return m
