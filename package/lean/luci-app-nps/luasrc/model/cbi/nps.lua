m=Map("nps")
m.title=translate("Nps Setting")
m.description=translate("Nps is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet.")

m:section(SimpleSection).template="nps/nps_status"

s=m:section(TypedSection,"nps")
s.addremove=false
s.anonymous=true

s:tab("basic",translate("Basic Setting"))
enable=s:taboption("basic",Flag,"enabled",translate("Enable"))
enable.rmempty=false
server=s:taboption("basic",Value,"server_addr",translate("Server"),translate("Must an IPv4 address"))
server.datatype="ipaddr"
server.optional=false
server.rmempty=false
port=s:taboption("basic",Value,"server_port",translate("Port"))
port.datatype="port"
port.default="8024"
port.optional=false
port.rmempty=false
protocol=s:taboption("basic",ListValue,"protocol",translate("Protocol Type"))
protocol.default="tcp"
protocol:value("tcp",translate("TCP Protocol"))
protocol:value("kcp",translate("KCP Protocol"))
vkey=s:taboption("basic",Value,"vkey",translate("vkey"))
vkey.optional=false
vkey.password=true
vkey.rmempty=false
compress=s:taboption("basic",Flag,"compress",translate("Enable Compression"),translate("The contents will be compressed to speed up the traffic forwarding speed, but this will consume some additional cpu resources."))
compress.default="1"
compress.rmempty=false
crypt=s:taboption("basic",Flag,"crypt",translate("Enable Encryption"),translate("Encrypted the communication between Npc and Nps, will effectively prevent the traffic intercepted."))
crypt.default="1"
crypt.rmempty=false
log_level=s:taboption("basic",ListValue,"log_level",translate("Log Level"))
log_level:value(0,"Emergency")
log_level:value(2,"Critical")
log_level:value(3,"Error")
log_level:value(4,"Warning")
log_level:value(7,"Debug")
log_level.default="3"

return m


