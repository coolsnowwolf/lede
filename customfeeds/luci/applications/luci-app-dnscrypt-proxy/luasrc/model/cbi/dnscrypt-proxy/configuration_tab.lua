-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local nxfs      = require("nixio.fs")
local util      = require("luci.util")
local uci_input = "/etc/config/dnscrypt-proxy"

if not nxfs.access(uci_input) then
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
	translate("This form allows you to modify the content of the main DNSCrypt-Proxy configuration file (/etc/config/dnscrypt-proxy)."))

f = s:option(TextValue, "data")
f.rows = 20
f.rmempty = true

function f.cfgvalue()
	return nxfs.readfile(uci_input) or ""
end

function f.write(self, section, data)
	return nxfs.writefile(uci_input, "\n" .. util.trim(data:gsub("\r\n", "\n")) .. "\n")
end

function s.handle(self, state, data)
	return true
end

return m
