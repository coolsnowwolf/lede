local fs = require "nixio.fs"
local jd = "jd-dailybonus"
local conffile = "/www/JD_DailyBonus.htm"

f = SimpleForm("logview")
m = Map(jd)
-- [[ 基本设置 ]]--

s = m:section(TypedSection, "global")
s.anonymous = true

t = s:option(TextValue, "1", nil)
t.rmempty = true
t.rows = 30
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end
t.readonly="readonly"

return m