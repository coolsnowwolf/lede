local fs = require "nixio.fs"
local log = "/var/log/dnsforwarder.log"

function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end
end

m = Map("dnsforwarder")
m.title = translate("运行日志")
m.description = translate("如果想观察日志，需要在配置文件中写明LogOn true 和 LogFileFolder /var/log")

s = m:section(TypedSection,"arguments","")
s.addremove = false
s.anonymous = true

gfwlist = s:option(TextValue, "gfwlist",nil, nil)
gfwlist.description = translate("日志文件默认位置:" .. log)
gfwlist.rows = 26
gfwlist.wrap = "off"
gfwlist.cfgvalue = function(self, section)
	return fs.readfile(log) or ""
end

return m
