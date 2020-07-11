local fs = require "nixio.fs"
local jd = "jd-dailybonus"
local uci = luci.model.uci.cursor()

function sync_value_to_file(value, file)
    value = value:gsub("\r\n?", "\n")
    local old_value = nixio.fs.readfile(file)
    if value ~= old_value then nixio.fs.writefile(file, value) end

end

s = SimpleForm("scriptview")

view_cfg = s:field(TextValue, "conf")
view_cfg.rmempty = false
view_cfg.rows = 43

function view_cfg.cfgvalue()
    return nixio.fs.readfile("/usr/share/jd-dailybonus/JD_DailyBonus.js") or ""
end
function view_cfg.write(self, section, value)
    sync_value_to_file(value, "/usr/share/jd-dailybonus/JD_DailyBonus.js")
end

return s
