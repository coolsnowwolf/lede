--[[
LuCI - Lua Configuration Interface

Copyright 2016 Weijie Gao <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

local sid = arg[1]
local utl = require "luci.util"

m = Map("vsftpd", translate("FTP Server - Virtual User &lt;new&gt;"))

m.redirect = luci.dispatcher.build_url("admin/nas/vsftpd/users")

if m.uci:get("vsftpd", sid) ~= "user" then
	luci.http.redirect(m.redirect)
	return
end

m.uci:foreach("vsftpd", "user",
	function(s)
		if s['.name'] == sid and s.username then
			m.title = translatef("FTP Server - Virtual User %q", s.username)
			return false
		end
	end)

s = m:section(NamedSection, sid, "settings", translate("User Settings"))
s.addremove = false

o = s:option(Value, "username", translate("Username"))
o.rmempty = false

function o.validate(self, value)
	if value == "" then
		return nil, translate("Username cannot be empty")
	end
	return value
end

o = s:option(Value, "password", translate("Password"))
o.password = true

o = s:option(Value, "home", translate("Home directory"))
o.default = "/home/ftp"

o = s:option(Value, "umask", translate("File mode umask"))
o.default = "022"

o = s:option(Value, "maxrate", translate("Max transmit rate"), translate("0 means no limitation"))
o.default = "0"

o = s:option(Flag, "writemkdir", translate("Enable write/mkdir"))
o.default = false

o = s:option(Flag, "upload", translate("Enable upload"))
o.default = false

o = s:option(Flag, "others", translate("Enable other rights"), translate("Include rename, deletion ..."))
o.default = false


return m
