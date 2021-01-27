local fs = require "nixio.fs"

log = SimpleForm("logview")
log.submit = false
log.reset = false

-- [[ 日志显示 ]]--
t = log:field(TextValue, "1", nil)
t.rmempty = true
t.rows = 30
function t.cfgvalue()
	return fs.readfile("/var/log/jd_dailybonus.log") or ""
end
t.readonly="readonly"

return log