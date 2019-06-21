-- Copyright (C) 2019 Anton Chen <contact@antonchen.com>
local sys = require "luci.sys"
local http = require "luci.http"
local dispatcher = require "luci.dispatcher"

m = Map("clash")
s = m:section(TypedSection, "subscription")
s.anonymous = true

o = s:option(Flag, "enabled", translate("Enable Subcription"))
o.default = 0
o.rmempty = false

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.default = 0
o.rmempty = false

o = s:option(Value, "url", translate("* Subcription Url"))
o.description = translate("Support V2Ray and Shadowsocks, but does not support shadowsocks plugin")
o.rmempty = true

o = s:option(Value, "pattern", translate("Group pattern"))
o.description = translate("Automatic grouping pattern, if the value is empty disabled")
o.rmempty = true

o = s:option(Value, "include", translate("Include"))
o.description = translate("Include node keyword, multiple value separated by commas")
o.rmempty = true

o = s:option(Value, "exclude", translate("Exclude"))
o.description = translate("Exclude node keyword, multiple value separated by commas")
o.rmempty = true

o = s:option(Button,"update", translate("Update Subcription"))
o.inputtitle = translate("Update")
o.inputstyle = "reload"
o.write = function()
    os.execute("sleep 5;/etc/init.d/clash restart >/dev/null 2>&1 &")
    http.redirect(dispatcher.build_url("admin", "services", "clash", "subscription"))
end

--- Apply
local apply = http.formvalue("cbi.apply")
if apply then
    os.execute("sleep 5;/etc/init.d/clash restart >/dev/null 2>&1 &")
end
return m
