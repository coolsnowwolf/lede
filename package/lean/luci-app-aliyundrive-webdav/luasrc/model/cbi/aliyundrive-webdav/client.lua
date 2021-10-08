local uci = luci.model.uci.cursor()
local m, e

m = Map("aliyundrive-webdav", translate("AliyunDrive WebDAV"),
        translate("<a href=\"https://github.com/messense/aliyundrive-webdav\" target=\"_blank\">Project GitHub URL</a>"))

m:section(SimpleSection).template = "aliyundrive-webdav/aliyundrive-webdav_status"

e = m:section(TypedSection, "server")
e.anonymous = true

enable = e:option(Flag, "enable", translate("Enable"))
enable.rmempty = false

refresh_token = e:option(Value, "refresh_token", translate("Refresh Token"),
        translate("<a href=\"https://github.com/messense/aliyundrive-webdav#%E8%8E%B7%E5%8F%96-refresh_token\" target=\"_blank\">How to get refresh token</a>"))

root = e:option(Value, "root", translate("Root Directory"))
root.default = "/"

host = e:option(Value, "host", translate("Host"))
host.default = "0.0.0.0"
host.datatype = "ipaddr"

port = e:option(Value, "port", translate("Port"))
port.default = "8080"
port.datatype = "port"

auth_user = e:option(Value, "auth_user", translate("Username"))
auth_password = e:option(Value, "auth_password", translate("Password"))
auth_password.password = true

read_buffer_size = e:option(Value, "read_buffer_size", translate("Read Buffer Size"))
read_buffer_size.default = "10485760"
read_buffer_size.datatype = "uinteger"

cache_size = e:option(Value, "cache_size", translate("Cache Size"))
cache_size.default = "1000"
cache_size.datatype = "uinteger"

debug = e:option(Flag, "debug", translate("Debug Mode"))
debug.rmempty = false

local e = luci.http.formvalue("cbi.apply")
if e then
    io.popen("/etc/init.d/aliyundrive-webdav restart")
end
return m
