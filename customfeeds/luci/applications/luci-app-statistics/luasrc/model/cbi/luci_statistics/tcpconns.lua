-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("TCPConns Plugin Configuration"),
	translate(
		"The tcpconns plugin collects informations about open tcp " ..
		"connections on selected ports."
	))

-- collectd_tcpconns config section
s = m:section( NamedSection, "collectd_tcpconns", "luci_statistics" )

-- collectd_tcpconns.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_tcpconns.listeningports (ListeningPorts)
listeningports = s:option( Flag, "ListeningPorts", translate("Monitor all local listen ports") )
listeningports.default = 1
listeningports:depends( "enable", 1 )

-- collectd_tcpconns.localports (LocalPort)
localports = s:option( Value, "LocalPorts", translate("Monitor local ports") )
localports.optional = true
localports:depends( "enable", 1 )

-- collectd_tcpconns.remoteports (RemotePort)
remoteports = s:option( Value, "RemotePorts", translate("Monitor remote ports") )
remoteports.optional = true
remoteports:depends( "enable", 1 )

return m
