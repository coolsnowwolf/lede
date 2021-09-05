-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("CSV Plugin Configuration"),
	translate(
		"The csv plugin stores collected data in csv file format " ..
		"for further processing by external programs."
	))

-- collectd_csv config section
s = m:section( NamedSection, "collectd_csv", "luci_statistics" )

-- collectd_csv.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_csv.datadir (DataDir)
datadir = s:option( Value, "DataDir", translate("Storage directory for the csv files") )
datadir.default = "127.0.0.1"
datadir:depends( "enable", 1 )

-- collectd_csv.storerates (StoreRates)
storerates = s:option( Flag, "StoreRates", translate("Store data values as rates instead of absolute values") )
storerates.default = 0
storerates:depends( "enable", 1 )

return m

