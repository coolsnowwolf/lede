-- Copyright (C) 2015 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local sshuser, server, port, ssh_options, identity, ipaddr, peeraddr

sshuser = section:taboption("general", Value, "sshuser", translate("SSH username"))

server = section:taboption("general", Value, "server", translate("SSH server address"))
server.datatype = "host(0)"

port = section:taboption("general", Value, "port", translate("SSH server port"))
port.datatype = "port"
port.optional = true
port.default = 22

ssh_options = section:taboption("general", Value, "ssh_options", translate("Extra SSH command options"))
ssh_options.optional = true

identity = section:taboption("general", DynamicList, "identity", translate("List of SSH key files for auth"))
identity.optional = true
identity.datatype = "file"

ipaddr = section:taboption("general", Value, "ipaddr", translate("Local IP address to assign"))
ipaddr.datatype = "ipaddr"

peeraddr = section:taboption("general", Value, "peeraddr", translate("Peer IP address to assign"))
peeraddr.datatype = "ipaddr"


local ipv6, defaultroute, metric, peerdns, dns,
      keepalive_failure, keepalive_interval, demand

if luci.model.network:has_ipv6() then
        ipv6 = section:taboption("advanced", Flag, "ipv6",
                translate("Enable IPv6 negotiation on the PPP link"))
        ipv6.default = ipv6.disabled
end


defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Use default gateway"),
	translate("If unchecked, no default route is configured"))

defaultroute.default = defaultroute.enabled


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"))

metric.placeholder = "0"
metric.datatype    = "uinteger"
metric:depends("defaultroute", defaultroute.enabled)


peerdns = section:taboption("advanced", Flag, "peerdns",
	translate("Use DNS servers advertised by peer"),
	translate("If unchecked, the advertised DNS server addresses are ignored"))

peerdns.default = peerdns.enabled


dns = section:taboption("advanced", DynamicList, "dns",
	translate("Use custom DNS servers"))

dns:depends("peerdns", "")
dns.datatype = "ipaddr"
dns.cast     = "string"


keepalive_failure = section:taboption("advanced", Value, "_keepalive_failure",
	translate("LCP echo failure threshold"),
	translate("Presume peer to be dead after given amount of LCP echo failures, use 0 to ignore failures"))

function keepalive_failure.cfgvalue(self, section)
	local v = m:get(section, "keepalive")
	if v and #v > 0 then
		return tonumber(v:match("^(%d+)[ ,]+%d+") or v)
	end
end

function keepalive_failure.write() end
function keepalive_failure.remove() end

keepalive_failure.placeholder = "0"
keepalive_failure.datatype    = "uinteger"


keepalive_interval = section:taboption("advanced", Value, "_keepalive_interval",
	translate("LCP echo interval"),
	translate("Send LCP echo requests at the given interval in seconds, only effective in conjunction with failure threshold"))

function keepalive_interval.cfgvalue(self, section)
	local v = m:get(section, "keepalive")
	if v and #v > 0 then
		return tonumber(v:match("^%d+[ ,]+(%d+)"))
	end
end

function keepalive_interval.write(self, section, value)
	local f = tonumber(keepalive_failure:formvalue(section)) or 0
	local i = tonumber(value) or 5
	if i < 1 then i = 1 end
	if f > 0 then
		m:set(section, "keepalive", "%d %d" %{ f, i })
	else
		m:set(section, "keepalive", "0")
	end
end

keepalive_interval.remove      = keepalive_interval.write
keepalive_interval.placeholder = "5"
keepalive_interval.datatype    = "min(1)"


demand = section:taboption("advanced", Value, "demand",
	translate("Inactivity timeout"),
	translate("Close inactive connection after the given amount of seconds, use 0 to persist connection"))

demand.placeholder = "0"
demand.datatype    = "uinteger"
