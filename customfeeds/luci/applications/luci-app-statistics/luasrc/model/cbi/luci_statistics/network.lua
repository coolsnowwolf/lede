-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("Network Plugin Configuration"),
	translate(
		"The network plugin provides network based communication between " ..
		"different collectd instances. Collectd can operate both in client " ..
		"and server mode. In client mode locally collected data is " ..
		"transferred to a collectd server instance, in server mode the " ..
		"local instance receives data from other hosts."
	))

-- collectd_network config section
s = m:section( NamedSection, "collectd_network", "luci_statistics" )

-- collectd_network.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0


-- collectd_network_listen config section (Listen)
listen = m:section( TypedSection, "collectd_network_listen",
	translate("Listener interfaces"),
	translate(
		"This section defines on which interfaces collectd will wait " ..
		"for incoming connections."
	))
listen.addremove = true
listen.anonymous = true

-- collectd_network_listen.host
listen_host = listen:option( Value, "host", translate("Listen host") )
listen_host.default = "0.0.0.0"

-- collectd_network_listen.port
listen_port = listen:option( Value, "port", translate("Listen port") )
listen_port.default   = 25826
listen_port.isinteger = true
listen_port.optional  = true


-- collectd_network_server config section (Server)
server = m:section( TypedSection, "collectd_network_server",
	translate("server interfaces"),
	translate(
		"This section defines to which servers the locally collected " ..
		"data is sent to."
	))
server.addremove = true
server.anonymous = true

-- collectd_network_server.host
server_host = server:option( Value, "host", translate("Server host") )
server_host.default = "0.0.0.0"

-- collectd_network_server.port
server_port = server:option( Value, "port", translate("Server port") )
server_port.default   = 25826
server_port.isinteger = true
server_port.optional  = true

-- collectd_network.timetolive (TimeToLive)
ttl = s:option( Value, "TimeToLive", translate("TTL for network packets") )
ttl.default   = 128
ttl.isinteger = true
ttl.optional  = true
ttl:depends( "enable", 1 )

-- collectd_network.forward (Forward)
forward = s:option( Flag, "Forward", translate("Forwarding between listen and server addresses") )
forward.default  = 0
forward.optional = true
forward:depends( "enable", 1 )

-- collectd_network.cacheflush (CacheFlush)
cacheflush = s:option( Value, "CacheFlush",
	translate("Cache flush interval"), translate("Seconds") )
cacheflush.default   = 86400
cacheflush.isinteger = true
cacheflush.optional  = true
cacheflush:depends( "enable", 1 )


return m
