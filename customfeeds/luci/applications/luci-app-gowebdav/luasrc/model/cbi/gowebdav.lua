-- Created By ImmortalWrt
-- https://github.com/immortalwrt

m = Map("gowebdav", translate("GoWebDav"), translate("GoWebDav is a tiny, simple, fast WevDav server."))

m:section(SimpleSection).template  = "gowebdav/gowebdav_status"

s = m:section(TypedSection, "gowebdav")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enable", translate("Enable"))
enable.rmempty = false

listen_port = s:option(Value, "listen_port", translate("Listen Port"))
listen_port.placeholder = 6086
listen_port.default     = 6086
listen_port.datatype    = "port"
listen_port.rmempty     = false

username = s:option(Value, "username", translate("Username"))
username.description = translate("Leave blank to disable auth.")
username.placeholder = user
username.default     = user
username.datatype    = "string"

password = s:option(Value, "password", translate("Password"))
password.description = translate("Leave blank to disable auth.")
password.placeholder = pass
password.default     = pass
password.datatype    = "string"
password.password    = true

root_dir = s:option(Value, "root_dir", translate("Root Directory"))
root_dir.placeholder = "/mnt"
root_dir.default     = "/mnt"
root_dir.rmempty     = false

read_only = s:option(Flag, "read_only", translate("Read-Only Mode"))
read_only.rmempty = false

allow_wan = s:option(Flag, "allow_wan", translate("Allow Access From Internet"))
allow_wan.rmempty = false

use_https = s:option(Flag, "use_https", translate("Use HTTPS instead of HTTP"))
use_https.rmempty = false

cert_cer = s:option(Value, "cert_cer", translate("Path to Certificate"))
cert_cer.datatype = "file"
cert_cer:depends("use_https", 1)

cert_key = s:option(Value, "cert_key", translate("Path to Certificate Key"))
cert_key.datatype = "file"
cert_key:depends("use_https", 1)

download_reg = s:option(DummyValue,"opennewwindow",translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"Download Reg File\" onclick=\"window.open('https://raw.githubusercontent.com/1715173329/gowebdav/master/allow_http.reg')\" />"))
download_reg.description = translate("Windows doesn't allow HTTP auth by default, you need to import this reg key to enable it (Reboot needed).")

return m
