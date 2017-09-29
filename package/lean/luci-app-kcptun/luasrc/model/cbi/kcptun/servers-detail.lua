-- Copyright 2016-2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local dsp = require "luci.dispatcher"

local m, s, o
local sid = arg[1]

local encrypt_methods = {
	"aes",
	"aes-128",
	"aes-192",
	"salsa20",
	"blowfish",
	"twofish",
	"cast5",
	"3des",
	"tea",
	"xtea",
	"xor",
	"none",
}

local modes = {
	"normal",
	"fast",
	"fast2",
	"fast3",
	"manual",
}

m = Map("kcptun", "%s - %s" % { translate("Kcptun"), translate("Edit Server") })
m.redirect = dsp.build_url("admin/services/kcptun/servers")

if m.uci:get("kcptun", sid) ~= "servers" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove = false

o = s:option(Value, "alias", "%s (%s)" % { translate("Alias"), translate("optional") })

o = s:option(Value, "server_addr", translate("Server"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.placeholder = "29900"

o = s:option(Value, "listen_addr", "%s (%s)" % { translate("Local Listen Host"), translate("optional") },
	translate("Local listen host."))
o.datatype = "host"
o.placeholder = "0.0.0.0"

o = s:option(Value, "listen_port", translate("Local Port"), translate("Local Listen Port."))
o.datatype = "port"
o.placeholder = "12948"

o = s:option(Value, "key", "%s (%s)" % { translate("Key"), translate("optional") },
	translate("Pre-shared secret for client and server."))
o.password = true
o.placeholder = "it's a secret"

o = s:option(Value, "crypt", translate("crypt"), translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods) do
	o:value(v, v:upper())
end
o.default = "aes"

o = s:option(ListValue, "mode", translate("mode"), translate("Embedded Mode"))
for _, v in ipairs(modes) do
	o:value(v, v:upper())
end
o.default = "fast"

o = s:option(Flag, "nodelay", translate("nodelay"), translate("Enable nodelay Mode."))
o:depends("mode", "manual")

o = s:option(Value, "interval", translate("interval"))
o:depends("mode", "manual")
o.datatype = "uinteger"
o.placeholder = "50"

o = s:option(ListValue, "resend", translate("resend"))
o:depends("mode", "manual")
o:value("0", translate("Off"))
o:value("1", translate("On"))
o:value("2", translate("2nd ACK"))

o = s:option(Flag, "nc", translate("nc"))
o:depends("mode", "manual")

o = s:option(Value, "mtu", "%s (%s)" % { translate("mtu"), translate("optional") },
	translate("Maximum transmission unit of UDP packets."))
o.datatype = "range(64,9200)"
o.placeholder = "1350"

o = s:option(Value, "sndwnd", "%s (%s)" % { translate("sndwnd"), translate("optional") },
	translate("Send Window Size(num of packets)."))
o.datatype = "min(1)"
o.default = "128"
o.placeholder = "128"

o = s:option(Value, "rcvwnd", "%s (%s)" % { translate("rcvwnd"), translate("optional") },
	translate("Receive Window Size(num of packets)."))
o.datatype = "min(1)"
o.default = "512"
o.placeholder = "512"

o = s:option(Value, "datashard", "%s (%s)" % { translate("datashard"), translate("optional") },
	translate("Reed-solomon Erasure Coding - datashard."))
o.datatype = "uinteger"
o.placeholder = "10"

o = s:option(Value, "parityshard", "%s (%s)" % { translate("parityshard"), translate("optional") },
	translate("Reed-solomon Erasure Coding - parityshard."))
o.datatype = "uinteger"
o.placeholder = "3"

o = s:option(Value, "dscp", "%s (%s)" % { translate("dscp"), translate("optional") }, translate("DSCP(6bit)"))
o.datatype = "uinteger"
o.placeholder = "0"

o = s:option(Flag, "nocomp", translate("nocomp"), translate("Disable Compression?"))
o.enabled = "true"
o.disabled = "false"
o.rmempty = false

o = s:option(Flag, "acknodelay", translate("acknodelay"))
o.enabled = "true"
o.disabled = "false"

o = s:option(Value, "conn", "%s (%s)" %{ translate("conn"), translate("optional") },
	translate("Number of UDP connections to server."))
o.datatype = "min(1)"
o.placeholder = "1"

o = s:option(Value, "autoexpire", "%s (%s)" % { translate("autoexpire"), translate("optional") },
	translate("Auto expiration time(in seconds) for a single UDP connection, 0 to disable."))
o.datatype = "uinteger"
o.placeholder = "0"

o = s:option(Value, "scavengettl", "%s (%s)" % { translate("scavengettl"), translate("optional") },
	translate("How long an expired connection can live(in sec), -1 to disable."))
o.datatype = "min(-1)"
o.placeholder = "600"

o = s:option(Value, "sockbuf", "%s (%s)" % { translate("sockbuf"), translate("optional") },
	translate("Send/secv buffer size of udp sockets, default unit is MB."))
o.datatype = "uinteger"
o.placeholder = "4"
o.cfgvalue = function(...)
	local value = Value.cfgvalue(...)

	if value then
		return tonumber(value) / 1024 / 1024
	end
end
o.write = function(self, section, value)
	local number = tonumber(value)
	if number then
		Value.write(self, section, number * 1024 * 1024)
	else
		Value.remove(self, section)
	end
end

o = s:option(Value, "keepalive", "%s (%s)" % { translate("keepalive"), translate("optional") },
	translate("NAT keepalive interval to prevent your router from removing port mapping, default unit is seconds."))
o.datatype = "uinteger"
o.placeholder = "10"

o = s:option(Value, "snmpperiod", "%s (%s)" % { translate("snmpperiod"),
	translate("optional") }, translate("SNMP collect period, in seconds"))
o.datatype = "min(1)"
o.placeholder = "60"

return m
