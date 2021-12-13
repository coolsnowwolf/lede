-- Copyright 2020 Lean <coolsnowwolf@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("ps3netsrv")
m.title = translate("PS3 NET Server")
m.description = translate("PS3NETSRV is ISO/PKG server for Sony PlayStation 3 webMAN-MOD plugin")

m:section(SimpleSection).template = "ps3netsrv/ps3netsrv_status"

s = m:section(TypedSection, "ps3netsrv")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enabled"))
enable.default = "0"
enable.rmempty = false

db_path = s:option(Value, "dir", translate("Dir Path"))
db_path.default = "/root"
db_path.rmempty = false

port = s:option(Value, "port", translate("Port"))
port.rmempty = false
port.datatype = "port"

return m
