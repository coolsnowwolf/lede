-- Copyright 2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local net = require "luci.model.network".init()
local qos = require "luci.model.qos_gargoyle"

local m, s, o
local upload_classes = {}
local download_classes = {}
local qos_gargoyle = "qos_gargoyle"

local function qos_enabled()
	return sys.init.enabled(qos_gargoyle)
end

uci:foreach(qos_gargoyle, "upload_class", function(s)
	local class_alias = s.name
	if class_alias then
		upload_classes[#upload_classes + 1] = {name = s[".name"], alias = class_alias}
	end
end)

uci:foreach(qos_gargoyle, "download_class", function(s)
	local class_alias = s.name
	if class_alias then
		download_classes[#download_classes + 1] = {name = s[".name"], alias = class_alias}
	end
end)

m = Map(qos_gargoyle, translate("Gargoyle QoS"),
	translate("Quality of Service (QoS) provides a way to control how available bandwidth is "
	.. "allocated."))

s = m:section(NamedSection, "global", "global", translate("Global Settings"))
s.anonymous = true

o = s:option(Button, "_switch", nil, translate("QoS Switch"))
o.render = function(self, section, scope)
	if qos_enabled() then
		self.title = translate("Disable QoS")
		self.inputstyle = "reset"
	else
		self.title = translate("Enable QoS")
		self.inputstyle = "apply"
	end
	Button.render(self, section, scope)
end
o.write = function(...)
	if qos_enabled() then
		sys.init.stop(qos_gargoyle)
		sys.init.disable(qos_gargoyle)
	else
		sys.init.enable(qos_gargoyle)
		sys.init.start(qos_gargoyle)
	end
end

s = m:section(NamedSection, "upload", "upload", translate("Upload Settings"))
s.anonymous = true

o = s:option(ListValue, "default_class", translate("Default Service Class"),
	translate("Specifie how packets that do not match any rule should be classified."))
for _, s in ipairs(upload_classes) do o:value(s.name, s.alias) end

o = s:option(Value, "total_bandwidth", translate("Total Upload Bandwidth"),
	translate("Should be set to around 98% of your available upload bandwidth. Entering a number "
	.. "which is too high will result in QoS not meeting its class requirements. Entering a number "
	.. "which is too low will needlessly penalize your upload speed. You should use a speed test "
	.. "program (with QoS off) to determine available upload bandwidth. Note that bandwidth is "
	.. "specified in kbps, leave blank to disable update QoS. There are 8 kilobits per kilobyte."))
o.datatype = "uinteger"

s = m:section(NamedSection, "download", "download", translate("Download Settings"))
s.anonymous = true

o = s:option(ListValue, "default_class", translate("Default Service Class"),
	translate("Specifie how packets that do not match any rule should be classified."))
for _, s in ipairs(download_classes) do o:value(s.name, s.alias) end

o = s:option(Value, "total_bandwidth", translate("Total Download Bandwidth"),
	translate("Specifying correctly is crucial to making QoS work. Note that bandwidth is specified "
	.. "in kbps, leave blank to disable download QoS. There are 8 kilobits per kilobyte."))
o.datatype = "uinteger"

o = s:option(Flag, "qos_monenabled", translate("Enable Active Congestion Control"),
	translate("<p>The active congestion control (ACC) observes your download activity and "
	.. "automatically adjusts your download link limit to maintain proper QoS performance. ACC "
	.. "automatically compensates for changes in your ISP's download speed and the demand from your "
	.. "network adjusting the link speed to the highest speed possible which will maintain proper QoS "
	.. "function. The effective range of this control is between 15% and 100% of the total download "
	.. "bandwidth you entered above.</p>") ..
	translate("<p>While ACC does not adjust your upload link speed you must enable and properly "
	.. "configure your upload QoS for it to function properly.</p>")
	)
o.enabled  = "true"
o.disabled = "false"

o = s:option(Value, "ptarget_ip", translate("Use Non-standard Ping Target"),
	translate("The segment of network between your router and the ping target is where congestion is "
	.. "controlled. By monitoring the round trip ping times to the target congestion is detected. By "
	.. "default ACC uses your WAN gateway as the ping target. If you know that congestion on your "
	.. "link will occur in a different segment then you can enter an alternate ping target. Leave "
	.. "empty to use the default settings."))
o:depends("qos_monenabled", "true")
local wan = qos.get_wan()
if wan then o:value(wan:gwaddr()) end
o.datatype = "ipaddr"

o = s:option(Value, "pinglimit", translate("Manual Ping Limit"),
	translate("Round trip ping times are compared against the ping limits. ACC controls the link "
	.. "limit to maintain ping times under the appropriate limit. By default ACC attempts to "
	.. "automatically select appropriate target ping limits for you based on the link speeds you "
	.. "entered and the performance of your link it measures during initialization. You cannot change "
	.. "the target ping time for the minRTT mode but by entering a manual time you can control the "
	.. "target ping time of the active mode. The time you enter becomes the increase in the target "
	.. "ping time between minRTT and active mode. Leave empty to use the default settings."))
o:depends("qos_monenabled", "true")
o:value("Auto", translate("Auto"))
o.datatype = "or('Auto', range(10, 250))"

return m
