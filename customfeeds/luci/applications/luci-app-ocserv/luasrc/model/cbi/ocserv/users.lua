-- Copyright 2014 Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local dsp = require "luci.dispatcher"
local nixio  = require "nixio"

m = Map("ocserv", translate("OpenConnect VPN"))

if m.uci:get("ocserv", "config", "auth") == "plain" then

--[[Users]]--

function m.on_commit(map)
	luci.sys.call("/etc/init.d/ocserv restart >/dev/null 2>&1")
end

s = m:section(TypedSection, "ocservusers", translate("Available users"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "name", translate("Name")).rmempty = true
s:option(DummyValue, "group", translate("Group")).rmempty = true
pwd = s:option(Value, "password", translate("Password"))
pwd.password = false

function pwd.write(self, section, value)
	local pass
	if string.match(value, "^\$%d\$.*") then
		pass = value
	else
		local t = tonumber(nixio.getpid()*os.time())
		local salt = "$1$" .. t .. "$"
		pass = nixio.crypt(value, salt)
	end
	Value.write(self, section, pass)
end	

--[[if plain]]--
end

local lusers = { }
local fd = io.popen("/usr/bin/occtl show users", "r")
if fd then local ln
	repeat
		ln = fd:read("*l")
		if not ln then break end

		local id, user, group, vpn_ip, ip, device, time, cipher, status = 
			ln:match("^%s*(%d+)%s+([-_%w]+)%s+([%(%)%.%*-_%w]+)%s+([%:%.-_%w]+)%s+([%:%.-_%w]+)%s+([%:%.-_%w]+)%s+([%:%.-_%w]+)%s+([%(%)%:%.-_%w]+)%s+([%:%.-_%w]+).*")
		if id then
			table.insert(lusers, {id, user, group, vpn_ip, ip, device, time, cipher, status})
		end
	until not ln
	fd:close()
end


--[[Active Users]]--

local s = m:section(Table, lusers, translate("Active users"))
s.anonymous = true
s.template = "cbi/tblsection"

s:option(DummyValue, 1, translate("ID"))
s:option(DummyValue, 2, translate("Username"))
s:option(DummyValue, 3, translate("Group"))
s:option(DummyValue, 4, translate("IP"))
s:option(DummyValue, 5, translate("VPN IP"))
s:option(DummyValue, 6, translate("Device"))
s:option(DummyValue, 7, translate("Time"))
s:option(DummyValue, 8, translate("Cipher"))
s:option(DummyValue, 9, translate("Status"))

return m
