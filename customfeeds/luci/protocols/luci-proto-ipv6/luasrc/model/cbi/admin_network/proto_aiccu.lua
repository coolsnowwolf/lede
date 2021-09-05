-- Copyright 2015 Paul Oranje <por@xs4all.nl>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

-- config read by /lib/netifd/proto/aiccu.sh
local username, password, protocol, server, tunnelid, ip6prefix, requiretls, nat, heartbeat,
	verbose, ntpsynctimeout, ip6addr, sourcerouting, defaultroute

-- generic parameters
local metric, ttl, mtu


username = section:taboption("general", Value, "username",
	translate("Server username"),
	translate("SIXXS-handle[/Tunnel-ID]"))
username.datatype = "string"

password = section:taboption("general", Value, "password",
	translate("Server password"),
	translate("Server password, enter the specific password of the tunnel when the username contains the tunnel ID"))
password.datatype = "string"
password.password = true


--[[ SIXXS supports only TIC as tunnel broker protocol, no use setting it.
protocol = section:taboption("general", ListValue, "protocol",
	translate("Tunnel broker protocol"),
	translate("SIXXS supports TIC only, for static tunnels using IP protocol 41 (RFC4213) use 6in4 instead"))

protocol:value("tic", "TIC")
protocol:value("tsp", "TSP")
protocol:value("l2tp", "L2TP")
protocol.default = "tic"
protocol.optional = true
--]]


server = section:taboption("general", Value, "server",
	translate("Tunnel setup server"),
	translate("Optional, specify to override default server (tic.sixxs.net)"))
server.datatype = "host(0)"
server.optional = true


tunnelid = section:taboption("general", Value, "tunnelid",
	translate("Tunnel ID"),
	translate("Optional, use when the SIXXS account has more than one tunnel"))
tunnelid.datatype = "string"
tunnelid.optional = true


local ip6prefix = section:taboption("general", Value, "ip6prefix",
	translate("IPv6 prefix"),
	translate("Routed IPv6 prefix for downstream interfaces"))
ip6prefix.datatype = "ip6addr"
ip6prefix.optional = true


heartbeat = s:taboption("general", ListValue, "heartbeat",
        translate("Tunnel type"),
	translate("Also see <a href=\"https://www.sixxs.net/faq/connectivity/?faq=comparison\">Tunneling Comparison</a> on SIXXS"))
heartbeat:value("0", translate("AYIYA"))
heartbeat:value("1", translate("Heartbeat"))
heartbeat.default = "0"


nat = section:taboption("general", Flag, "nat",
	translate("Behind NAT"),
	translate("The tunnel end-point is behind NAT, defaults to disabled and only applies to AYIYA"))
nat.optional = true
nat.default = nat.disabled


requiretls = section:taboption("general", Flag, "requiretls",
	translate("Require TLS"),
	translate("Connection to server fails when TLS cannot be used"))
requiretls.optional = true
requiretls.default = requiretls.disabled


verbose = section:taboption("advanced", Flag, "verbose",
	translate("Verbose"),
	translate("Verbose logging by aiccu daemon"))
verbose.optional = true
verbose.default = verbose.disabled


ntpsynctimeout = section:taboption("advanced", Value, "ntpsynctimeout",
	translate("NTP sync time-out"),
	translate("Wait for NTP sync that many seconds, seting to 0 disables waiting (optional)"))
ntpsynctimeout.datatype = "uinteger"
ntpsynctimeout.placeholder = "90"
ntpsynctimeout.optional = true


ip6addr = section:taboption("advanced", Value, "ip6addr",
	translate("Local IPv6 address"),
	translate("IPv6 address delegated to the local tunnel endpoint (optional)"))
ip6addr.datatype = "ip6addr"
ip6addr.optional = true


defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Default route"),
	translate("Whether to create an IPv6 default route over the tunnel"))
defaultroute.default = defaultroute.enabled
defaultroute.optional = true


sourcerouting = section:taboption("advanced", Flag, "sourcerouting",
	translate("Source routing"),
	translate("Whether to route only packets from delegated prefixes"))
sourcerouting.default = sourcerouting.enabled
sourcerouting.optional = true


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"))
metric.datatype = "uinteger"
metric.placeholder = "0"
metric:depends("defaultroute", defaultroute.enabled)


ttl = section:taboption("advanced", Value, "ttl",
	translate("Use TTL on tunnel interface"))
ttl.datatype = "range(1,255)"
ttl.placeholder = "64"


mtu = section:taboption("advanced", Value, "mtu",
	translate("Use MTU on tunnel interface"),
        translate("minimum 1280, maximum 1480"))
mtu.datatype = "range(1280,1480)"
mtu.placeholder = "1280"

