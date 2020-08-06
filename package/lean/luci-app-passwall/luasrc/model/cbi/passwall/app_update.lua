local d = require "luci.dispatcher"
local appname = "passwall"

m = Map(appname)

-- [[ App Settings ]]--
s = m:section(TypedSection, "global_app", translate("App Update"),
              "<font color='red'>" ..
                  translate("Please confirm that your firmware supports FPU.") ..
                  "</font>")
s.anonymous = true
s:append(Template(appname .. "/app_update/v2ray_version"))
s:append(Template(appname .. "/app_update/trojan_go_version"))
s:append(Template(appname .. "/app_update/kcptun_version"))
s:append(Template(appname .. "/app_update/brook_version"))

---- V2ray Path
o = s:option(Value, "v2ray_file", translate("V2ray Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/v2ray/"))
o.default = "/usr/bin/v2ray/"
o.rmempty = false

---- Trojan-Go Path
o = s:option(Value, "trojan_go_file", translate("Trojan-Go Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/trojan-go"))
o.default = "/usr/bin/trojan-go"
o.rmempty = false

o = s:option(Value, "trojan_go_latest", translate("Trojan-Go Version API"), translate("alternate API URL for version checking"))
o.default = "https://api.github.com/repos/peter-tank/trojan-go/releases/latest"

---- Kcptun client Path
o = s:option(Value, "kcptun_client_file", translate("Kcptun Client Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/kcptun-client"))
o.default = "/usr/bin/kcptun-client"
o.rmempty = false

--[[
o = s:option(Button,  "_check_kcptun",  translate("Manually update"), translatef("Make sure there is enough space to install %s", "kcptun"))
o.template = appname .. "/kcptun"
o.inputstyle = "apply"
o.btnclick = "onBtnClick_kcptun(this);"
o.id = "_kcptun-check_btn"]] --

---- Brook Path
o = s:option(Value, "brook_file", translate("Brook Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/brook"))
o.default = "/usr/bin/brook"
o.rmempty = false

return m
