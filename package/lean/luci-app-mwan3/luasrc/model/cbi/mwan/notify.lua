-- Copyright 2014 Aedan Renner <chipdankly@gmail.com>
-- Copyright 2018 Florian Eckert <fe@dev.tdt.de>
-- Licensed to the public under the GNU General Public License v2.

local fs = require "nixio.fs"
local ut = require "luci.util"
local script = "/etc/mwan3.user"

local m, f, t

m = SimpleForm("luci", translate("MWAN - Notification"))

f = m:section(SimpleSection, nil,
	translate("This section allows you to modify the content of \"/etc/mwan3.user\".<br />" ..
	"The file is also preserved during sysupgrade.<br />" ..
	"<br />" ..
	"Notes:<br />" ..
	"This file is interpreted as a shell script.<br />" ..
	"The first line of the script must be &#34;#!/bin/sh&#34; without quotes.<br />" ..
	"Lines beginning with # are comments and are not executed.<br />" ..
	"Put your custom mwan3 action here, they will<br />" ..
	"be executed with each netifd hotplug interface event<br />" ..
	"on interfaces for which mwan3 is enabled.<br />" ..
	"<br />" ..
	"There are three main environment variables that are passed to this script.<br />" ..
	"<br />" ..
	"$ACTION <br />" ..
	"* \"ifup\" Is called by netifd and mwan3track <br />" ..
	"* \"ifdown\" Is called by netifd and mwan3track <br />" ..
	"* \"connected\" Is only called by mwan3track if tracking was successful <br />" ..
	"* \"disconnected\" Is only called by mwan3track if tracking has failed <br />" ..
	"$INTERFACE Name of the interface which went up or down (e.g. \"wan\" or \"wwan\")<br />" ..
	"$DEVICE Physical device name which interface went up or down (e.g. \"eth0\" or \"wwan0\")<br />" ..
	"<br />"))

t = f:option(TextValue, "lines")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
	return fs.readfile(script)
end
function t.write(self, section, data)
	return fs.writefile(script, ut.trim(data:gsub("\r\n", "\n")) .. "\n")
end

return m
