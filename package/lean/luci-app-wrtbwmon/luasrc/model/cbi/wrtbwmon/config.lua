m = Map("wrtbwmon")
m.title = translate("Details")

s = m:section(NamedSection, "general", "wrtbwmon", translate("General settings"))

o = s:option(Flag, "persist", translate("Persist database"))
o.description = translate("Check this to persist the database file")
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
