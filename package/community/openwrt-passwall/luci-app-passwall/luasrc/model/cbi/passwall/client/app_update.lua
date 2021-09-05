local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

m = Map(appname)

-- [[ App Settings ]]--
s = m:section(TypedSection, "global_app", translate("App Update"),
              "<font color='red'>" ..
                  translate("Please confirm that your firmware supports FPU.") ..
                  "</font>")
s.anonymous = true
s:append(Template(appname .. "/app_update/v2ray_version"))
s:append(Template(appname .. "/app_update/xray_version"))
s:append(Template(appname .. "/app_update/trojan_go_version"))
s:append(Template(appname .. "/app_update/kcptun_version"))
s:append(Template(appname .. "/app_update/brook_version"))
s:append(Template(appname .. "/app_update/hysteria_version"))

o = s:option(Value, "v2ray_file", translatef("%s App Path", "V2ray"))
o.default = "/usr/bin/v2ray"
o.rmempty = false

o = s:option(Value, "xray_file", translatef("%s App Path", "Xray"))
o.default = "/usr/bin/xray"
o.rmempty = false

o = s:option(Value, "trojan_go_file", translatef("%s App Path", "Trojan-Go"))
o.default = "/usr/bin/trojan-go"
o.rmempty = false

o = s:option(Value, "kcptun_client_file", translatef("%s Client App Path", "Kcptun"))
o.default = "/usr/bin/kcptun-client"
o.rmempty = false

o = s:option(Value, "brook_file", translatef("%s App Path", "Brook"))
o.default = "/usr/bin/brook"
o.rmempty = false

o = s:option(Value, "hysteria_file", translatef("%s App Path", "Hysteria"))
o.default = "/usr/bin/hysteria"
o.rmempty = false

o = s:option(DummyValue, "tips", " ")
o.rawhtml = true
o.cfgvalue = function(t, n)
    return string.format('<font color="red">%s</font>', translate("if you want to run from memory, change the path, /tmp beginning then save the application and update it manually."))
end

return m
