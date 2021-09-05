-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("luci_statistics",
	translate("IRQ Plugin Configuration"),
	translate(
		"The irq plugin will monitor the rate of issues per second for " ..
		"each selected interrupt. If no interrupt is selected then all " ..
		"interrupts are monitored."
	))

-- collectd_irq config section
s = m:section( NamedSection, "collectd_irq", "luci_statistics" )

-- collectd_irq.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0

-- collectd_irq.irqs (Irq)
irqs = s:option( Value, "Irqs", translate("Monitor interrupts") )
irqs.optional = true
irqs:depends( "enable", 1 )

-- collectd_irq.ignoreselected (IgnoreSelected)
ignoreselected = s:option( Flag, "IgnoreSelected", translate("Monitor all except specified") )
ignoreselected.default  = 0
ignoreselected.optional = "true"
ignoreselected:depends( "enable", 1 )

return m
