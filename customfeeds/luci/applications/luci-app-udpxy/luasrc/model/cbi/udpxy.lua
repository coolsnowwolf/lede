-- Copyright 2014 Álvaro Fernández Rojas <noltari@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("udpxy", "udpxy", translate("udpxy is a UDP-to-HTTP multicast traffic relay daemon, here you can configure the settings."))

s = m:section(TypedSection, "udpxy", "")
s.addremove = true
s.anonymous = false

enable=s:option(Flag, "disabled", translate("Enabled"))
enable.enabled="0"
enable.disabled="1"
enable.default = "1"
enable.rmempty = false
respawn=s:option(Flag, "respawn", translate("Respawn"))
respawn.default = false

verbose=s:option(Flag, "verbose", translate("Verbose"))
verbose.default = false

status=s:option(Flag, "status", translate("Status"))

bind=s:option(Value, "bind", translate("Bind IP/Interface"))
bind.rmempty = true
bind.datatype = "or(ipaddr, network)"

port=s:option(Value, "port", translate("Port"))
port.rmempty = true
port.datatype = "port"

source=s:option(Value, "source", translate("Source IP/Interface"))
source.rmempty = true
source.datatype = "or(ipaddr, network)"

max_clients=s:option(Value, "max_clients", translate("Max clients"))
max_clients.rmempty = true
max_clients.datatype = "range(1, 5000)"

log_file=s:option(Value, "log_file", translate("Log file"))
log_file.rmempty = true
--log_file.datatype = "file"

buffer_size=s:option(Value, "buffer_size", translate("Buffer size"))
buffer_size.rmempty = true
buffer_size.datatype = "range(4096,2097152)"

buffer_messages=s:option(Value, "buffer_messages", translate("Buffer messages"))
buffer_messages.rmempty = true
buffer_messages.datatype = "or(-1, and(min(1), uinteger))"

buffer_time=s:option(Value, "buffer_time", translate("Buffer time"))
buffer_time.rmempty = true
buffer_time.datatype = "or(-1, and(min(1), uinteger))"

nice_increment=s:option(Value, "nice_increment", translate("Nice increment"))
nice_increment.rmempty = true
nice_increment.datatype = "or(and(max(-1), integer),and(min(1), integer))"

mcsub_renew=s:option(Value, "mcsub_renew", translate("Multicast subscription renew"))
mcsub_renew.rmempty = true
mcsub_renew.datatype = "or(0, range(30, 64000))"

return m
