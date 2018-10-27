--[[
 Customize firewall-banned domain lists - /etc/gfwlist/
 Copyright (c) 2015 Justin Liu
 Author: Justin Liu <rssnsj@gmail.com>
 https://github.com/rssnsj/network-feeds
]]--

local fs = require "nixio.fs"

function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end
end

m = SimpleForm("gfwlist", translate("Domain Lists Settings"))

for e in fs.dir("/etc/gfwlist") do
	glist = m:field(TextValue, e, e, nil)
	glist.rmempty = false
	glist.rows = 12

	function glist.cfgvalue()
		return nixio.fs.readfile("/etc/gfwlist/" .. e) or ""
	end
	function glist.write(self, section, value)
		sync_value_to_file(value, "/etc/gfwlist/" .. e)
	end
end

return m
