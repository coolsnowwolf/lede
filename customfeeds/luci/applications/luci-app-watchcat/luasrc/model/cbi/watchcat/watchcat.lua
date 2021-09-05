-- Copyright 2012 Christian Gagneraud <chris@techworks.ie>
-- Licensed to the public under the Apache License 2.0.

m = Map("system", 
	translate("Watchcat"), 
	translate("Watchcat allows configuring a periodic reboot when the " ..
		  "Internet connection has been lost for a certain period of time."
		 ))

s = m:section(TypedSection, "watchcat")
s.anonymous = true
s.addremove = true

mode = s:option(ListValue, "mode",
		translate("Operating mode"))
mode.default = "allways"
mode:value("ping", "Reboot on internet connection lost")
mode:value("allways", "Periodic reboot")

forcedelay = s:option(Value, "forcedelay",
		      translate("Forced reboot delay"),
		      translate("When rebooting the system, the watchcat will trigger a soft reboot. " ..
				"Entering a non zero value here will trigger a delayed hard reboot " ..
				"if the soft reboot fails. Enter a number of seconds to enable, " ..
				"use 0 to disable"))
forcedelay.datatype = "uinteger"
forcedelay.default = "0"

period = s:option(Value, "period", 
		  translate("Period"),
		  translate("In periodic mode, it defines the reboot period. " ..
			    "In internet mode, it defines the longest period of " .. 
			    "time without internet access before a reboot is engaged." ..
			    "Default unit is seconds, you can use the " ..
			    "suffix 'm' for minutes, 'h' for hours or 'd' " ..
			    "for days"))

pinghost = s:option(Value, "pinghosts", 
		    translate("Ping host"),
		    translate("Host address to ping"))
pinghost.datatype = "host(1)"
pinghost.default = "8.8.8.8"
pinghost:depends({mode="ping"})

pingperiod = s:option(Value, "pingperiod", 
		      translate("Ping period"),
		      translate("How often to check internet connection. " ..
				"Default unit is seconds, you can you use the " ..
				"suffix 'm' for minutes, 'h' for hours or 'd' " ..
				"for days"))
pingperiod:depends({mode="ping"})

return m
