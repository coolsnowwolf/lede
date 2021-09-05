-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.util")


m = Map("system", translate("System"), translate("Here you can configure the basic aspects of your device like its hostname or the timezone."))

s = m:section(TypedSection, "system", "")
s.anonymous = true
s.addremove = false


local sysinfo = luci.util.ubus("system", "info") or { }
local boardinfo = luci.util.ubus("system", "board") or { }

local uptime = sysinfo.uptime or 0
local loads = sysinfo.load or { 0, 0, 0 }
local memory = sysinfo.memory or {
	total = 0,
	free = 0,
	buffered = 0,
	shared = 0
}

s:option(DummyValue, "_system", translate("Model")).value = boardinfo.model or "?"
s:option(DummyValue, "_cpu", translate("System")).value = boardinfo.system or "?"

s:option(DummyValue, "_la", translate("Load")).value =
 string.format("%.2f, %.2f, %.2f", loads[1] / 65535.0, loads[2] / 65535.0, loads[3] / 65535.0)

s:option(DummyValue, "_memtotal", translate("Memory")).value =
 string.format("%.2f MB (%.0f%% %s, %.0f%% %s)",
  tonumber(memory.total) / 1024 / 1024,
  100 * memory.buffered / memory.total,
  tostring(translate("buffered")),
  100 * memory.free / memory.total,
  tostring(translate("free"))
)

s:option(DummyValue, "_systime", translate("Local Time")).value =
 os.date("%c")

s:option(DummyValue, "_uptime", translate("Uptime")).value =
 luci.tools.webadmin.date_format(tonumber(uptime))

hn = s:option(Value, "hostname", translate("Hostname"))

function hn.write(self, section, value)
	Value.write(self, section, value)
	luci.sys.hostname(value)
end


tz = s:option(ListValue, "zonename", translate("Timezone"))
tz:value("UTC")

for i, zone in ipairs(luci.sys.zoneinfo.TZ) do
        tz:value(zone[1])
end

function tz.write(self, section, value)
        local function lookup_zone(title)
                for _, zone in ipairs(luci.sys.zoneinfo.TZ) do
                        if zone[1] == title then return zone[2] end
                end
        end

        AbstractValue.write(self, section, value)
        self.map.uci:set("system", section, "timezone", lookup_zone(value) or "GMT0")
end

return m
