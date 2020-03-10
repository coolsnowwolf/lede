
local SYS  = require "luci.sys"
local ND = SYS.exec("cat /usr/share/adbyby/dnsmasq.adblock | wc -l")

m = Map("adbyby")

s = m:section(TypedSection, "adbyby")
s.anonymous = true

o = s:option(Flag, "cron_mode")
o.title = translate("Update the rule at 6 a.m. every morning and restart adbyby")
o.default = 0
o.rmempty = false
o.description = string.format("<strong>"..translate("ADP Host List")..":</strong> %s", ND)

updatead = s:option(Button, "updatead", translate("Manually force update<br />Adblock Plus Host List"), translate("Note: It needs to download and convert the rules. The background process may takes 60-120 seconds to run. <br / > After completed it would automatically refresh, please do not duplicate click!"))
updatead.inputtitle = translate("Manually force update")
updatead.inputstyle = "apply"
updatead.write = function()
	SYS.call("sh /usr/share/adbyby/adblock.sh > /tmp/adupdate.log 2>&1 &")
end

o = s:option(Flag, "block_ios")
o.title = translate("Block Apple iOS OTA update")
o.default = 0
o.rmempty = false

return m