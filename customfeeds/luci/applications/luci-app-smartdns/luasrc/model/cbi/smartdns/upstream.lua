--
-- Copyright (C) 2018-2020 Ruilin Peng (Nick) <pymumu@gmail.com>.
--
-- smartdns is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- smartdns is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

local sid = arg[1]

m = Map("smartdns", "%s - %s" %{translate("SmartDNS Server"), translate("Upstream DNS Server Configuration")})
m.redirect = luci.dispatcher.build_url("admin/services/smartdns")

if m.uci:get("smartdns", sid) ~= "server" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Edit Server ]]--
s = m:section(NamedSection, sid, "server")
s.anonymous = true
s.addremove   = false

---- name
s:option(Value, "name", translate("DNS Server Name"), translate("DNS Server Name"))

---- IP address
o = s:option(Value, "ip", translate("ip"), translate("DNS Server ip"))
o.datatype = "or(host, string)"
o.rmempty = false 
---- port
o = s:option(Value, "port", translate("port"), translate("DNS Server port"))
o.placeholder = "default"
o.datatype    = "port"
o.rempty      = true
o:depends("type", "udp")
o:depends("type", "tcp")
o:depends("type", "tls")

---- type
o = s:option(ListValue, "type", translate("type"), translate("DNS Server type"))
o.placeholder = "udp"
o:value("udp", translate("udp"))
o:value("tcp", translate("tcp"))
o:value("tls", translate("tls"))
o:value("https", translate("https"))
o.default     = "udp"
o.rempty      = false

---- TLS host verify
o = s:option(Value, "tls_host_verify", translate("TLS Hostname Verify"), translate("Set TLS hostname to verify."))
o.default     = ""
o.datatype    = "string"
o.rempty      = true
o:depends("type", "tls")
o:depends("type", "https")

---- Certificate verify
o = s:option(Flag, "no_check_certificate", translate("No check certificate"), translate("Do not check certificate."))
o.default = o.disabled
o.rmempty = false
o:depends("type", "tls")
o:depends("type", "https")
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "0"
end

---- SNI host name
o = s:option(Value, "host_name", translate("TLS SNI name"), translate("Sets the server name indication for query."))
o.default     = ""
o.datatype    = "hostname"
o.rempty      = true
o:depends("type", "tls")
o:depends("type", "https")

---- http host
o = s:option(Value, "http_host", translate("HTTP Host"), translate("Set the HTTP host used for the query. Use this parameter when the host of the URL address is an IP address."))
o.default     = ""
o.datatype    = "hostname"
o.rempty      = true
o:depends("type", "https")

---- server group
o = s:option(Value, "server_group", translate("Server Group"), translate("DNS Server group belongs to, used with nameserver, such as office, home."))
o.rmempty     = true
o.placeholder = "default"
o.datatype    = "hostname"
o.rempty      = true

---- blacklist_ip
o = s:option(Flag, "blacklist_ip", translate("IP Blacklist Filtering"), translate("Filtering IP with blacklist"))
o.rmempty     = false
o.default     = o.disabled
o.cfgvalue    = function(...)
    return Flag.cfgvalue(...) or "0"
end

---- anti-Answer-Forgery
-- o = s:option(Flag, "check_edns", translate("Anti Answer Forgery"), translate("Anti answer forgery, if DNS does not work properly after enabling, please turn off this feature"))
-- o.rmempty     = false
-- o.default     = o.disabled
-- o:depends("type", "udp")
-- o.cfgvalue    = function(...)
--     return Flag.cfgvalue(...) or "0"
-- end

---- SPKI pin
o = s:option(Value, "spki_pin", translate("TLS SPKI Pinning"), translate("Used to verify the validity of the TLS server, The value is Base64 encoded SPKI fingerprint, leaving blank to indicate that the validity of TLS is not verified."))
o.default     = ""
o.datatype    = "string"
o.rempty      = true
o:depends("type", "tls")
o:depends("type", "https")


---- other args
o = s:option(Value, "addition_arg", translate("Additional Server Args"), translate("Additional Args for upstream dns servers"))
o.default     = ""
o.rempty      = true
o.optional    = true

return m
