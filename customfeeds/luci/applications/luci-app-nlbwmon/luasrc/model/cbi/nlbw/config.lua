-- Copyright 2017 Jo-Philipp Wich <jo@mein.io>
-- Licensed to the public under the Apache License 2.0.

local utl = require "luci.util"
local sys = require "luci.sys"
local fs  = require "nixio.fs"
local ip  = require "luci.ip"
local nw  = require "luci.model.network"

local s, m, period, warning, date, days, interval, ifaces, subnets, limit, prealloc, compress, generations, commit, refresh, directory, protocols

m = Map("nlbwmon", translate("Netlink Bandwidth Monitor - Configuration"),
	translate("The Netlink Bandwidth Monitor (nlbwmon) is a lightweight, efficient traffic accounting program keeping track of bandwidth usage per host and protocol."))

nw.init(luci.model.uci.cursor_state())

s = m:section(TypedSection, "nlbwmon")
s.anonymous = true
s.addremove = false
s:tab("general", translate("General Settings"))
s:tab("advanced", translate("Advanced Settings"))
s:tab("protocol", translate("Protocol Mapping"),
	translate("Protocol mappings to distinguish traffic types per host, one mapping per line. The first value specifies the IP protocol, the second value the port number and the third column is the name of the mapped protocol."))

period = s:taboption("general", ListValue, "_period", translate("Accounting period"),
	translate("Choose \"Day of month\" to restart the accounting period monthly on a specific date, e.g. every 3rd. Choose \"Fixed interval\" to restart the accounting period exactly every N days, beginning at a given date."))

period:value("relative", translate("Day of month"))
period:value("absolute", translate("Fixed interval"))

period.write = function(self, cfg, val)
	if period:formvalue(cfg) == "relative" then
		m:set(cfg, "database_interval", interval:formvalue(cfg))
	else
		m:set(cfg, "database_interval", "%s/%s" %{
			date:formvalue(cfg),
			days:formvalue(cfg)
		})
	end
end

period.cfgvalue = function(self, cfg)
	local val = m:get(cfg, "database_interval") or ""
	if val:match("^%d%d%d%d%-%d%d%-%d%d/%d+$") then
		return "absolute"
	end
	return "relative"
end


warning = s:taboption("general", DummyValue, "_warning", translate("Warning"))
warning.default = translatef("Changing the accounting interval type will invalidate existing databases!<br /><strong><a href=\"%s\">Download backup</a></strong>.", luci.dispatcher.build_url("admin/nlbw/backup"))
warning.rawhtml = true

if (m.uci:get_first("nlbwmon", "nlbwmon", "database_interval") or ""):match("^%d%d%d%d-%d%d-%d%d/%d+$") then
	warning:depends("_period", "relative")
else
	warning:depends("_period", "absolute")
end


interval = s:taboption("general", Value, "_interval", translate("Due date"),
	translate("Day of month to restart the accounting period. Use negative values to count towards the end of month, e.g. \"-5\" to specify the 27th of July or the 24th of Februrary."))

interval.datatype = "or(range(1,31),range(-31,-1))"
interval.placeholder = "1"
interval:value("1", translate("1 - Restart every 1st of month"))
interval:value("-1", translate("-1 - Restart every last day of month"))
interval:value("-7", translate("-7 - Restart a week before end of month"))
interval.rmempty = false
interval:depends("_period", "relative")
interval.write = period.write

interval.cfgvalue = function(self, cfg)
	local val = m:get(cfg, "database_interval")
	return val and tonumber(val)
end


date = s:taboption("general", Value, "_date", translate("Start date"),
	translate("Start date of the first accounting period, e.g. begin of ISP contract."))

date.datatype = "dateyyyymmdd"
date.placeholder = "2016-03-15"
date.rmempty = false
date:depends("_period", "absolute")
date.write = period.write

date.cfgvalue = function(self, cfg)
	local val = m:get(cfg, "database_interval") or ""
	return (val:match("^(%d%d%d%d%-%d%d%-%d%d)/%d+$"))
end


days = s:taboption("general", Value, "_days", translate("Interval"),
	translate("Length of accounting interval in days."))

days.datatype = "min(1)"
days.placeholder = "30"
days.rmempty = false
days:depends("_period", "absolute")
days.write = period.write

days.cfgvalue = function(self, cfg)
	local val = m:get(cfg, "database_interval") or ""
	return (val:match("^%d%d%d%d%-%d%d%-%d%d/(%d+)$"))
end


ifaces = s:taboption("general", Value, "_ifaces", translate("Local interfaces"),
	translate("Only conntrack streams from or to any of these networks are counted."))

ifaces.template = "cbi/network_netlist"
ifaces.widget = "checkbox"
ifaces.nocreate = true

ifaces.cfgvalue = function(self, cfg)
	return m:get(cfg, "local_network")
end

ifaces.write = function(self, cfg)
	local item
	local items = {}
	for item in utl.imatch(subnets:formvalue(cfg)) do
		items[#items+1] = item
	end
	for item in utl.imatch(ifaces:formvalue(cfg)) do
		items[#items+1] = item
	end
	m:set(cfg, "local_network", items)
end


subnets = s:taboption("general", DynamicList, "_subnets", translate("Local subnets"),
	translate("Only conntrack streams from or to any of these subnets are counted."))

subnets.datatype = "ipaddr"

subnets.cfgvalue = function(self, cfg)
	local subnet
	local subnets = {}
	for subnet in utl.imatch(m:get(cfg, "local_network")) do
		subnet = ip.new(subnet)
		subnets[#subnets+1] = subnet and subnet:string()
	end
	return subnets
end

subnets.write = ifaces.write


limit = s:taboption("advanced", Value, "database_limit", translate("Maximum entries"),
	translate("The maximum amount of entries that should be put into the database, setting the limit to 0 will allow databases to grow indefinitely."))

limit.datatype = "uinteger"
limit.placeholder = "10000"

prealloc = s:taboption("advanced", Flag, "database_prealloc", translate("Preallocate database"),
	translate("Whether to preallocate the maximum possible database size in memory. This is mainly useful for memory constrained systems which might not be able to satisfy memory allocation after longer uptime periods."))

prealloc:depends({["database_limit"] = "0", ["!reverse"] = true })


compress = s:taboption("advanced", Flag, "database_compress", translate("Compress database"),
	translate("Whether to gzip compress archive databases. Compressing the database files makes accessing old data slightly slower but helps to reduce storage requirements."))

compress.default = compress.enabled


generations = s:taboption("advanced", Value, "database_generations", translate("Stored periods"),
	translate("Maximum number of accounting periods to keep, use zero to keep databases forever."))

generations.datatype = "uinteger"
generations.placeholder = "10"


commit = s:taboption("advanced", Value, "commit_interval", translate("Commit interval"),
	translate("Interval at which the temporary in-memory database is committed to the persistent database directory."))

commit.placeholder = "24h"
commit:value("24h", translate("24h - least flash wear at the expense of data loss risk"))
commit:value("12h", translate("12h - compromise between risk of data loss and flash wear"))
commit:value("10m", translate("10m - frequent commits at the expense of flash wear"))
commit:value("60s", translate("60s - commit minutely, useful for non-flash storage"))


refresh = s:taboption("advanced", Value, "refresh_interval", translate("Refresh interval"),
	translate("Interval at which traffic counters of still established connections are refreshed from netlink information."))

refresh.placeholder = "30s"
refresh:value("30s", translate("30s - refresh twice per minute for reasonably current stats"))
refresh:value("5m", translate("5m - rarely refresh to avoid frequently clearing conntrack counters"))


directory = s:taboption("advanced", Value, "database_directory", translate("Database directory"),
	translate("Database storage directory. One file per accounting period will be placed into this directory."))

directory.placeholder = "/var/lib/nlbwmon"


protocols = s:taboption("protocol", TextValue, "_protocols")
protocols.rows = 50

protocols.cfgvalue = function(self, cfg)
	return fs.readfile("/usr/share/nlbwmon/protocols")
end

protocols.write = function(self, cfg, value)
	fs.writefile("/usr/share/nlbwmon/protocols", (value or ""):gsub("\r\n", "\n"))
end

protocols.remove = protocols.write


return m
