local e = require "nixio.fs"
local e = require "luci.sys"

m = Map("passwall")
-- [[ Rule Settings ]]--
s = m:section(TypedSection, "global_rules", translate("Rule status"))
s.anonymous = true
s:append(Template("passwall/rule/rule_version"))

--[[
o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false
]]--

---- Auto Update
o = s:option(Flag, "auto_update", translate("Enable auto update rules"))
o.default = 0
o.rmempty = false

---- Week Update
o = s:option(ListValue, "week_update", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update", 1)

---- Time Update
o = s:option(ListValue, "time_update", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update", 1)

-- [[ App Settings ]]--
s = m:section(TypedSection, "global_app", translate("App Update"),
              "<font color='red'>" ..
                  translate("Please confirm that your firmware supports FPU.") ..
                  "</font>")
s.anonymous = true
s:append(Template("passwall/rule/passwall_version"))
s:append(Template("passwall/rule/v2ray_version"))
s:append(Template("passwall/rule/kcptun_version"))
s:append(Template("passwall/rule/brook_version"))

---- V2ray Path
o = s:option(Value, "v2ray_file", translate("V2ray Path"), translate(
                 "if you want to run from memory, change the path, such as /tmp/v2ray/, Then save the application and update it manually."))
o.default = "/usr/bin/v2ray/"
o.rmempty = false

---- Kcptun client Path
o = s:option(Value, "kcptun_client_file", translate("Kcptun Client Path"),
             translate(
                 "if you want to run from memory, change the path, such as /tmp/kcptun-client, Then save the application and update it manually."))
o.default = "/usr/bin/kcptun-client"
o.rmempty = false

--[[
o = s:option(Button,  "_check_kcptun",  translate("Manually update"), translate("Make sure there is enough space to install Kcptun"))
o.template = "passwall/kcptun"
o.inputstyle = "apply"
o.btnclick = "onBtnClick_kcptun(this);"
o.id = "_kcptun-check_btn"]] --

---- Brook Path
o = s:option(Value, "brook_file", translate("Brook Path"), translate(
                 "if you want to run from memory, change the path, such as /tmp/brook, Then save the application and update it manually."))
o.default = "/usr/bin/brook"
o.rmempty = false

return m
