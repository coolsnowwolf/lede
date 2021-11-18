m = Map("nps")
m.title = translate("Nps")
m.description = translate("Nps is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet.")

m:section(SimpleSection).template = "nps/nps_status"

s = m:section(TypedSection, "nps")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"))
enable.rmempty = false

server = s:option(Value, "server_addr", translate("Server"))
server.description = translate("Must an IPv4 address")
server.datatype = "ipaddr"
server.optional = false
server.rmempty = false

port = s:option(Value, "server_port", translate("Port"))
port.datatype = "port"
port.default = "8024"
port.optional = false
port.rmempty = false

protocol = s:option(ListValue, "protocol", translate("Protocol Type"))
protocol:value("tcp", translate("TCP Protocol"))
protocol:value("kcp", translate("KCP Protocol"))
protocol.default = "tcp"

vkey = s:option(Value, "vkey", translate("vkey"))
vkey.optional = false
vkey.password = true
vkey.rmempty = false

compress = s:option(Flag, "compress", translate("Enable Compression"))
compress.description = translate("The contents will be compressed to speed up the traffic forwarding speed, but this will consume some additional cpu resources.")
compress.default = "1"
compress.rmempty = false

crypt = s:option(Flag, "crypt", translate("Enable Encryption"))
crypt.description = translate("Encrypted the communication between Npc and Nps, will effectively prevent the traffic intercepted.")
crypt.default = "1"
crypt.rmempty = false

log_level = s:option(ListValue,"log_level",translate("Log Level"))
log_level:value(0,"Emergency", translate("Emergency"))
log_level:value(2,"Critical", translate("Critical"))
log_level:value(3,"Error", translate("Error"))
log_level:value(4,"Warning", translate("Warning"))
log_level:value(7,"Debug", translate("Debug"))
log_level.default = "3"

return m
