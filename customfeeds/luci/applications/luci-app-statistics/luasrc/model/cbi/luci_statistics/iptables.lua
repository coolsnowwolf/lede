-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

require("luci.sys.iptparser")

ip = luci.sys.iptparser.IptParser()
chains  = { }
targets = { }

for i, rule in ipairs( ip:find() ) do 
	if rule.chain and rule.target then
		chains[rule.chain] = true
		targets[rule.target] = true
	end
end


m = Map("luci_statistics",
	translate("Iptables Plugin Configuration"),
	translate(
		"The iptables plugin will monitor selected firewall rules and " ..
		"collect informations about processed bytes and packets per rule."
	))

-- collectd_iptables config section
s = m:section( NamedSection, "collectd_iptables", "luci_statistics" )

-- collectd_iptables.enable
enable = s:option( Flag, "enable", translate("Enable this plugin") )
enable.default = 0


-- collectd_iptables_match config section (Chain directives)
rule = m:section( TypedSection, "collectd_iptables_match",
	translate("Add matching rule"),
	translate(
		"Here you can define various criteria by which the monitored " ..
		"iptables rules are selected."
	))
rule.addremove = true
rule.anonymous = true


-- collectd_iptables_match.name
rule_table = rule:option( Value, "name",
	translate("Name of the rule"), translate("max. 16 chars") )

-- collectd_iptables_match.table
rule_table = rule:option( ListValue, "table", translate("Table") )
rule_table.default  = "filter"
rule_table.rmempty  = true
rule_table.optional = true
rule_table:value("")
rule_table:value("filter")
rule_table:value("nat")
rule_table:value("mangle")


-- collectd_iptables_match.chain
rule_chain = rule:option( ListValue, "chain", translate("Chain") )
rule_chain.rmempty  = true
rule_chain.optional = true
rule_chain:value("")

for chain, void in pairs( chains ) do
	rule_chain:value( chain )
end


-- collectd_iptables_match.target
rule_target = rule:option( ListValue, "target", translate("Action (target)") )
rule_target.rmempty  = true
rule_target.optional = true
rule_target:value("")

for target, void in pairs( targets ) do
	rule_target:value( target )
end


-- collectd_iptables_match.protocol
rule_protocol = rule:option( ListValue, "protocol", translate("Network protocol") )
rule_protocol.rmempty  = true
rule_protocol.optional = true
rule_protocol:value("")
rule_protocol:value("tcp")
rule_protocol:value("udp")
rule_protocol:value("icmp")

-- collectd_iptables_match.source
rule_source = rule:option( Value, "source", translate("Source ip range") )
rule_source.default  = "0.0.0.0/0"
rule_source.rmempty  = true
rule_source.optional = true

-- collectd_iptables_match.destination
rule_destination = rule:option( Value, "destination", translate("Destination ip range") )
rule_destination.default  = "0.0.0.0/0"
rule_destination.rmempty  = true
rule_destination.optional = true

-- collectd_iptables_match.inputif
rule_inputif = rule:option( Value, "inputif",
	translate("Incoming interface"), translate("e.g. br-lan") )
rule_inputif.rmempty  = true
rule_inputif.optional = true

-- collectd_iptables_match.outputif
rule_outputif = rule:option( Value, "outputif",
	translate("Outgoing interface"), translate("e.g. br-ff") )
rule_outputif.rmempty  = true
rule_outputif.optional = true

-- collectd_iptables_match.options
rule_options = rule:option( Value, "options",
	translate("Options"), translate("e.g. reject-with tcp-reset") )
rule_options.rmempty  = true
rule_options.optional = true

return m
