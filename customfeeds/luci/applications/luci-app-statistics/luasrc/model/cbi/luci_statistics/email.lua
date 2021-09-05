-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("E-Mail Plugin Configuration"),
	translate(
		"The email plugin creates a unix socket which can be used " ..
		"to transmit email-statistics to a running collectd daemon. " ..
		"This plugin is primarily intended to be used in conjunction " ..
		"with Mail::SpamAssasin::Plugin::Collectd but can be used in " ..
		"other ways as well."
	))

-- collectd_email config section
s = m:section( NamedSection, "collectd_email", "luci_statistics" )

-- collectd_email.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_email.socketfile (SocketFile)
socketfile = s:option( Value, "SocketFile", translate("Socket file") )
socketfile.default = "/var/run/collect-email.sock"
socketfile:depends( "enable", 1 )

-- collectd_email.socketgroup (SocketGroup)
socketgroup = s:option( Value, "SocketGroup", translate("Socket group") )
socketgroup.default  = "nobody"
socketgroup.rmempty  = true
socketgroup.optional = true
socketgroup:depends( "enable", 1 )

-- collectd_email.socketperms (SocketPerms)
socketperms = s:option( Value, "SocketPerms", translate("Socket permissions") )
socketperms.default  = "0770"
socketperms.rmempty  = true
socketperms.optional = true
socketperms:depends( "enable", 1 )

-- collectd_email.maxconns (MaxConns)
maxconns = s:option( Value, "MaxConns", translate("Maximum allowed connections") )
maxconns.default   = 5
maxconns.isinteger = true
maxconns.rmempty   = true
maxconns.optional  = true
maxconns:depends( "enable", 1 )

return m
