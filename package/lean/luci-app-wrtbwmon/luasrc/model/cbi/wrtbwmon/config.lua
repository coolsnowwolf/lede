local m = Map("wrtbwmon", "详细设置")

local s = m:section(NamedSection, "general", "wrtbwmon", "通用设置")

local o = s:option(Flag, "persist", "写入数据库到 ROM",
    "把统计数据写入 /etc/config 中避免重启或者升级后丢失 （需要占用 ROM 空间并降低闪存寿命）")
o.rmempty = false

function o.write(self, section, value)
    if value == '1' then
        luci.sys.call("mv /tmp/usage.db /etc/config/usage.db")
    elseif value == '0' then
        luci.sys.call("mv /etc/config/usage.db /tmp/usage.db")
    end
    return Flag.write(self, section ,value)
end

return m
