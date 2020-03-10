
local SYS  = require "luci.sys"
local ND = SYS.exec("cat /usr/share/adbyby/dnsmasq.adblock | wc -l")

local ad_count=0
if nixio.fs.access("/usr/share/adbyby/dnsmasq.adblock") then
ad_count=tonumber(SYS.exec("cat /usr/share/adbyby/dnsmasq.adblock | wc -l"))
end

m = Map("adbyby")

s = m:section(TypedSection, "adbyby")
s.anonymous = true

o = s:option(Flag, "cron_mode")
o.title = translate("Update the rule at 6 a.m. every morning and restart adbyby")
o.default = 0
o.rmempty = false

o=s:option(DummyValue,"ad_data",translate("Adblock Plus Data"))
o.rawhtml  = true
o.template = "adbyby/refresh"
o.value =ad_count .. " " .. translate("Records")

o = s:option(Flag, "block_ios")
o.title = translate("Block Apple iOS OTA update")
o.default = 0
o.rmempty = false

return m