-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("RRDTool Plugin Configuration"),
	translate(
		"The rrdtool plugin stores the collected data in rrd database " ..
		"files, the foundation of the diagrams.<br /><br />" ..
		"<strong>Warning: Setting the wrong values will result in a very " ..
		"high memory consumption in the temporary directory. " ..
		"This can render the device unusable!</strong>"
	))

-- collectd_rrdtool config section
s = m:section( NamedSection, "collectd_rrdtool", "luci_statistics" )

-- collectd_rrdtool.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 1

-- collectd_rrdtool.datadir (DataDir)
datadir = s:option( Value, "DataDir",
	translate("Storage directory"),
	translate("Note: as pages are rendered by user 'nobody', the *.rrd files, " ..
		  "the storage directory and all its parent directories need " ..
		  "to be world readable."
	))
datadir.default  = "/tmp"
datadir.rmempty  = true
datadir.optional = true
datadir:depends( "enable", 1 )

-- collectd_rrdtool.stepsize (StepSize)
stepsize = s:option( Value, "StepSize",
	translate("RRD step interval"), translate("Seconds") )
stepsize.default   = 30
stepsize.isinteger = true
stepsize.rmempty   = true
stepsize.optional  = true
stepsize:depends( "enable", 1 )

-- collectd_rrdtool.heartbeat (HeartBeat)
heartbeat = s:option( Value, "HeartBeat",
	translate("RRD heart beat interval"), translate("Seconds") )
heartbeat.default   = 60
heartbeat.isinteger = true
heartbeat.rmempty   = true
heartbeat.optional  = true
heartbeat:depends( "enable", 1 )

-- collectd_rrdtool.rrasingle (RRASingle)
rrasingle = s:option( Flag, "RRASingle",
	translate("Only create average RRAs"), translate("reduces rrd size") )
rrasingle.default  = true
rrasingle:depends( "enable", 1 )

-- collectd_rrdtool.rramax (RRAMax)
rramax = s:option( Flag, "RRAMax",
	translate("Show max values instead of averages"),
	translate("Max values for a period can be used instead of averages when not using 'only average RRAs'") )
rramax.default  = false
rramax.rmempty  = true
rramax:depends( "RRASingle", 0 )

-- collectd_rrdtool.rratimespans (RRATimespan)
rratimespans = s:option( Value, "RRATimespans",
	translate("Stored timespans"), translate("seconds; multiple separated by space") )
rratimespans.default  = "600 86400 604800 2678400 31622400"
rratimespans.rmempty  = true
rratimespans.optional = true
rratimespans:depends( "enable", 1 )

-- collectd_rrdtool.rrarows (RRARows)
rrarows = s:option( Value, "RRARows", translate("Rows per RRA") )
rrarows.isinteger = true
rrarows.default   = 100
rrarows.rmempty   = true
rrarows.optional  = true
rrarows:depends( "enable", 1 )

-- collectd_rrdtool.xff (XFF)
xff = s:option( Value, "XFF", translate("RRD XFiles Factor") )
xff.default  = 0.1
xff.isnumber = true
xff.rmempty  = true
xff.optional = true
xff:depends( "enable", 1 )

-- collectd_rrdtool.cachetimeout (CacheTimeout)
cachetimeout = s:option( Value, "CacheTimeout",
	translate("Cache collected data for"), translate("Seconds") )
cachetimeout.isinteger = true
cachetimeout.default   = 100
cachetimeout.rmempty   = true
cachetimeout.optional  = true
cachetimeout:depends( "enable", 1 )

-- collectd_rrdtool.cacheflush (CacheFlush)
cacheflush = s:option( Value, "CacheFlush",
	translate("Flush cache after"), translate("Seconds") )
cacheflush.isinteger = true
cacheflush.default   = 100
cacheflush.rmempty   = true
cacheflush.optional  = true
cacheflush:depends( "enable", 1 )

return m
