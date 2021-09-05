-- Copyright 2014 Nikos Mavrogiannopoulos <nmav@gnutls.org>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local server, username, password, cert, ca
local oc_cert_file, oc_key_file, oc_ca_file

local ifc = net:get_interface():name()

oc_cert_file = "/etc/openconnect/user-cert-" .. ifc .. ".pem"
oc_key_file = "/etc/openconnect/user-key-" .. ifc .. ".pem"
oc_ca_file = "/etc/openconnect/ca-" .. ifc .. ".pem"

server = section:taboption("general", Value, "server", translate("VPN Server"))
server.datatype = "host(0)"

port = section:taboption("general", Value, "port", translate("VPN Server port"))
port.placeholder = "443"
port.datatype    = "port"


defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Use default gateway"),
	translate("If unchecked, no default route is configured"))

defaultroute.default = defaultroute.enabled


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"))

metric.placeholder = "0"
metric.datatype    = "uinteger"
metric:depends("defaultroute", defaultroute.enabled)

section:taboption("general", Value, "serverhash", translate("VPN Server's certificate SHA1 hash"))

section:taboption("general", Value, "authgroup", translate("Auth Group"))

username = section:taboption("general", Value, "username", translate("Username"))
password = section:taboption("general", Value, "password", translate("Password"))
password.password = true
password2 = section:taboption("general", Value, "password2", translate("Password2"))
password2.password = true


cert = section:taboption("advanced", Value, "usercert", translate("User certificate (PEM encoded)"))
cert.template = "cbi/tvalue"
cert.rows = 10

function cert.cfgvalue(self, section)
	return nixio.fs.readfile(oc_cert_file)
end

function cert.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(oc_cert_file, value)
end

cert = section:taboption("advanced", Value, "userkey", translate("User key (PEM encoded)"))
cert.template = "cbi/tvalue"
cert.rows = 10

function cert.cfgvalue(self, section)
	return nixio.fs.readfile(oc_key_file)
end

function cert.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(oc_key_file, value)
end


ca = section:taboption("advanced", Value, "ca", translate("CA certificate; if empty it will be saved after the first connection."))
ca.template = "cbi/tvalue"
ca.rows = 10

function ca.cfgvalue(self, section)
	return nixio.fs.readfile(oc_ca_file)
end

function ca.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(oc_ca_file, value)
end

mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"))
mtu.placeholder = "1406"
mtu.datatype    = "max(9200)"
