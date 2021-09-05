-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local fs = require("nixio.fs")
local util = require("luci.util")
local uci = require("luci.model.uci").cursor()
local adbinput = uci:get("adblock", "blacklist", "adb_src") or "/etc/adblock/adblock.blacklist"

if not nixio.fs.access(adbinput) then
	m = SimpleForm("error", nil,
		translate("Input file not found, please check your configuration."))
	m.reset = false
	m.submit = false
	return m
end

if nixio.fs.stat(adbinput).size > 524288 then
	m = SimpleForm("error", nil,
		translate("The file size is too large for online editing in LuCI (&gt; 512 KB). ")
		.. translate("Please edit this file directly in a terminal session."))
	m.reset = false
	m.submit = false
	return m
end

m = SimpleForm("input", nil)
m:append(Template("adblock/config_css"))
m.submit = translate("Save")
m.reset = false

s = m:section(SimpleSection, nil,
	translatef("This form allows you to modify the content of the adblock blacklist (%s).<br />", adbinput)
	.. translate("Please add only one domain per line. Comments introduced with '#' are allowed - ip addresses, wildcards and regex are not."))

f = s:option(TextValue, "data")
f.datatype = "string"
f.rows = 20
f.rmempty = true

function f.cfgvalue()
	return nixio.fs.readfile(adbinput) or ""
end

function f.write(self, section, data)
	return nixio.fs.writefile(adbinput, "\n" .. util.trim(data:gsub("\r\n", "\n")) .. "\n")
end

function s.handle(self, state, data)
	return true
end

return m
