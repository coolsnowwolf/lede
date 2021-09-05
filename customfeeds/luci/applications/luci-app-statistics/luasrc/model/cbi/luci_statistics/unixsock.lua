-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("Unixsock Plugin Configuration"),
	translate(
		"The unixsock plugin creates a unix socket which can be used " ..
		"to read collected data from a running collectd instance."
	))

-- collectd_unixsock config section
s = m:section( NamedSection, "collectd_unixsock", "luci_statistics" )

-- collectd_unixsock.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_unixsock.socketfile (SocketFile)
socketfile = s:option( Value, "SocketFile" )
socketfile.default = "/var/run/collect-query.socket"
socketfile:depends( "enable", 1 )

-- collectd_unixsock.socketgroup (SocketGroup)
socketgroup = s:option( Value, "SocketGroup" )
socketgroup.default  = "nobody"
socketgroup.rmempty  = true
socketgroup.optional = true
socketgroup:depends( "enable", 1 )

-- collectd_unixsock.socketperms (SocketPerms)
socketperms = s:option( Value, "SocketPerms" )
socketperms.default  = "0770"
socketperms.rmempty  = true
socketperms.optional = true
socketperms:depends( "enable", 1 )

return m
