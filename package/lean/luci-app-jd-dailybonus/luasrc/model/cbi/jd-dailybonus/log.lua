local fs = require "nixio.fs"
local jd = "jd-dailybonus"
local conffile = "/www/JD_DailyBonus.htm"

log = SimpleForm("logview")
log.submit = false
log.reset = false

-- [[ 日志显示 ]]--
t = log:field(TextValue, "1", nil)
t.rmempty = true
t.rows = 30
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end
t.readonly="readonly"

return log