-- Copyright (C) 2021 jerryk <jerrykuku@qq.com>
-- This is free software, licensed under the APACHE LICENSE, VERSION 2.0.


local uci = luci.model.uci.cursor()
local m, e

m = Map("go-aliyundrive-webdav")
m.title = translate("Go AliyunDrive WebDAV")
m.description = translate("<a href=\"https://github.com/LinkLeong/go-aliyun-webdav\" target=\"_blank\">Project GitHub URL</a>")

m:section(SimpleSection).template = "go-aliyundrive-webdav/go-aliyundrive-webdav_status"

e = m:section(TypedSection, "go-aliyundrive-webdav")
e.anonymous = true

enable = e:option(Flag, "enabled", translate("Enabled"))
enable.rmempty = false

rt_token = e:option(Value, "rt", translate("Refresh Token"))
rt_token.description = translate("<a href=\"https://github.com/LinkLeong/go-aliyun-webdav#%E6%B5%8F%E8%A7%88%E5%99%A8%E8%8E%B7%E5%8F%96refreshtoken%E6%96%B9%E5%BC%8F\" target=\"_blank\">How to get refresh token</a>")

port = e:option(Value, "port", translate("Port"))
port.default = "8085"
port.datatype = "port"

path = e:option(Value, "path", translate("Mounting directory"))
path.description = translate("Access aliyundrive directory, defaults /.")
path.default = "/"

user = e:option(Value, "user", translate("Username"))
user.default = "admin"

pwd = e:option(Value, "pwd", translate("Password"))
pwd.password = true
pwd.default = "123456"

verbose = e:option(Flag, "verbose", translate("Enable detailed logging"))
verbose.default = "0"
verbose.rmempty = false
verbose.optional = false


local e = luci.http.formvalue("cbi.apply")
if e then
	io.popen("/etc/init.d/go-aliyundrive-webdav restart")
end

return m
