--[[
LuCI - Lua Configuration Interface

Copyright 2016 Weijie Gao <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

m = Map("vsftpd", translate("FTP Server - Virtual User Settings"))

sv = m:section(NamedSection, "vuser", "vuser", translate("Settings"))

o = sv:option(Flag, "enabled", translate("Enabled"))
o.default = false

o = sv:option(Value, "username", translate("Username"), translate("An actual local user to handle virtual users"))
o.default = "ftp"

s = m:section(TypedSection, "user", translate("User lists"))
s.template = "cbi/tblsection"
s.extedit  = luci.dispatcher.build_url("admin/nas/vsftpd/item/%s")
s.addremove = true
s.anonymous = true

function s.create(...)
	local id = TypedSection.create(...)
	luci.http.redirect(s.extedit % id)
end

function s.remove(self, section)
	return TypedSection.remove(self, section)
end

o = s:option(DummyValue, "username", translate("Username"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...) or ("<%s>" % translate("Unknown"))
	return v
end
o.rmempty = false

o = s:option(DummyValue, "home", translate("Home directory"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...) or ("/home/ftp")
	return v
end
o.rmempty = false

return m
