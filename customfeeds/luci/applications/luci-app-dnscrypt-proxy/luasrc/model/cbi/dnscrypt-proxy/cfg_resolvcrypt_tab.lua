-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local nxfs      = require("nixio.fs")
local util      = require("luci.util")
local res_input = "/etc/resolv-crypt.conf"

if not nxfs.access(res_input) then
	m = SimpleForm("error", nil, translate("Input file not found, please check your configuration."))
	m.reset = false
	m.submit = false
	return m
end

m = SimpleForm("input", nil)
m:append(Template("dnscrypt-proxy/config_css"))
m.submit = translate("Save")
m.reset = false

s = m:section(SimpleSection, nil,
	translate("This form allows you to modify the content of the resolv-crypt configuration file (/etc/resolv-crypt.conf)."))

f = s:option(TextValue, "data")
f.rows = 20
f.rmempty = true

function f.cfgvalue()
	return nxfs.readfile(res_input) or ""
end

function f.write(self, section, data)
	return nxfs.writefile(res_input, "\n" .. util.trim(data:gsub("\r\n", "\n")) .. "\n")
end

function s.handle(self, state, data)
	return true
end

return m
