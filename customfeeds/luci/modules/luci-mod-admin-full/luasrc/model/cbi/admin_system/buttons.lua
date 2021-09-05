-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("system", translate("Buttons"),
	translate("This page allows the configuration of custom button actions"))

s = m:section(TypedSection, "button", "")
s.anonymous = true
s.addremove = true

s:option(Value, "button", translate("Name"))

act = s:option(ListValue, "action",
	translate("Action"),
	translate("Specifies the button state to handle"))

act:value("released")

s:option(Value, "handler",
	translate("Handler"),
	translate("Path to executable which handles the button event"))

min = s:option(Value, "min", translate("Minimum hold time"))
min.rmempty = true

max = s:option(Value, "max", translate("Maximum hold time"))
max.rmempty = true
