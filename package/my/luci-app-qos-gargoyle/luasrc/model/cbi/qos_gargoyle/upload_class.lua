-- Copyright 2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local m, s, o
local sid = arg[1]
local qos_gargoyle = "qos_gargoyle"

m = Map(qos_gargoyle, translate("Edit Upload Service Class"))
m.redirect = luci.dispatcher.build_url("admin/network/qos_gargoyle/upload")

if m.uci:get(qos_gargoyle, sid) ~= "upload_class" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, sid, "upload_class")
s.anonymous = true
s.addremove = false

o = s:option(Value, "name", translate("Service Class Name"))
o.rmempty = false

o = s:option(Value, "percent_bandwidth", translate("Percent Bandwidth At Capacity"),
	translate("The percentage of the total available bandwidth that should be allocated to this class "
	.. "when all available bandwidth is being used. If unused bandwidth is available, more can (and "
	.. "will) be allocated. The percentages can be configured to equal more (or less) than 100, but "
	.. "when the settings are applied the percentages will be adjusted proportionally so that they "
	.. "add to 100. This setting only comes into effect when the WAN link is saturated."))
o.datatype = "range(1, 100)"
o.rmempty  = false

o = s:option(Value, "min_bandwidth", translate("Minimum Bandwidth"),
	translate("The minimum service this class will be allocated when the link is at capacity. Classes "
	.. "which specify minimum service are known as realtime classes by the active congestion "
	.. "controller. Streaming video, VoIP and interactive online gaming are all examples of "
	.. "applications that must have a minimum bandwith to function. To determine what to enter use "
	.. "the application on an unloaded LAN and observe how much bandwidth it uses. Then enter a "
	.. "number only slightly higher than this into this field. QoS will satisfiy the minimum service "
	.. "of all classes first before allocating to other waiting classes so be careful to use minimum "
	.. "bandwidths sparingly."))
o:value("0", translate("Zero"))
o.datatype = "uinteger"
o.default  = "0"

o = s:option(Value, "max_bandwidth", translate("Maximum Bandwidth"),
	translate("The maximum amount of bandwidth this class will be allocated in kbit/s. Even if unused "
	.. "bandwidth is available, this service class will never be permitted to use more than this "
	.. "amount of bandwidth."))
o:value("", translate("Unlimited"))
o.datatype = "uinteger"

return m
