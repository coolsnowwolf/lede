-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

local fs = require("nixio.fs")

m = Map("uhttpd")
m.title = translate("Web Admin Settings")
m.description = translate("Web Admin Settings Page")

ucs = m:section(TypedSection, "uhttpd")
ucs.addremove = false
ucs.anonymous = true

lhttp = ucs:option(DynamicList, "listen_http", translate("HTTP listeners (address:port)"))
lhttp.description = translate("Bind to specific interface:port (by specifying interface address")
lhttp.datatype = "list(ipaddrport(1))"

function lhttp.validate(self, value, section)
        local have_https_listener = false
        local have_http_listener = false
	if lhttp and lhttp:formvalue(section) and (#(lhttp:formvalue(section)) > 0) then
		for k, v in pairs(lhttp:formvalue(section)) do
			if v and (v ~= "") then
				have_http_listener = true
				break
			end
		end
	end
	if lhttps and lhttps:formvalue(section) and (#(lhttps:formvalue(section)) > 0) then
		for k, v in pairs(lhttps:formvalue(section)) do
			if v and (v ~= "") then
				have_https_listener = true
				break
			end
		end
	end
	if not (have_http_listener or have_https_listener) then
		return nil, "must listen on at list one address:port"
	end
	return DynamicList.validate(self, value, section)
end

o = ucs:option(Flag, "redirect_https", translate("Redirect all HTTP to HTTPS"))
o.description = translate("Redirect all HTTP to HTTPS when SSl cert was installed")
o.default = o.enabled
o.rmempty = false

o = ucs:option(Flag, "rfc1918_filter", translate("Ignore private IPs on public interface"))
o.description = translate("Prevent access from private (RFC1918) IPs on an interface if it has an public IP address")
o.default = o.enabled
o.rmempty = false

return m
