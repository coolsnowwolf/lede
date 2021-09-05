-- Copyright 2016 Roger Pueyo Centelles <roger.pueyo@guifi.net>
-- Licensed to the public under the Apache License 2.0.

local map, section, net = ...

local peeraddr, ipaddr, ttl, tos, df, mtu, tunlink

peeraddr = section:taboption("general", Value, "peeraddr", translate("Remote IPv4 address or FQDN"), translate("The IPv4 address or the fully-qualified domain name of the remote tunnel end."))
peeraddr.optional = false
peeraddr.datatype = "or(hostname,ip4addr)"

ipaddr = section:taboption("general", Value, "ipaddr", translate("Local IPv4 address"), translate("The local IPv4 address over which the tunnel is created (optional)."))
ipaddr.optional = true
ipaddr.datatype = "ip4addr"

tunlink = section:taboption("general", Value, "tunlink", translate("Bind interface"), translate("Bind the tunnel to this interface (optional)."))
ipaddr.optional = true


mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"), translate("Specify an MTU (Maximum Transmission Unit) other than the default (1280 bytes)."))
mtu.optional = true
mtu.placeholder = 1280
mtu.datatype = "range(68, 9200)"

ttl = section:taboption("advanced", Value, "ttl", translate("Override TTL"), translate("Specify a TTL (Time to Live) for the encapsulating packet other than the default (64)."))
ttl.optional = true
ttl.placeholder = 64
ttl.datatype = "min(1)"

tos = section:taboption("advanced", Value, "tos", translate("Override TOS"), translate("Specify a TOS (Type of Service)."))
tos.optional = true
tos.datatype = "range(0, 255)"

df = section:taboption("advanced", Flag, "df", translate("Don't Fragment"), translate("Enable the DF (Don't Fragment) flag of the encapsulating packets."))
