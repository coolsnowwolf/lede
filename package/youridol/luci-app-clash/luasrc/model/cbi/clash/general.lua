-- Copyright (C) 2019 Anton Chen <contact@antonchen.com>
local http = require "luci.http"
local dispatcher = require "luci.dispatcher"

function rndLetter(num)
    local ret = ""
    pcall(function()
        for var = 1,num do
            if math.random()>0.5 then
                ret = ret..string.char(math.random(65,90))
            else
                ret = ret..string.char(math.random(97,122))
            end
        end
    end)
    return ret
end

math.randomseed(tonumber(tostring(os.time()):reverse():sub(1,6)))

m = Map("clash")
s = m:section(TypedSection, "general")
s.anonymous = true

o = s:option(DummyValue, "_status", translate("Status"))
o.value = "<span id=\"_clash\" style=\"line-height: 2.1em;\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(DummyValue, "_china_network", translate("China Network"))
o.value = "<span id=\"_china_network\" style=\"line-height: 2.1em;\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(DummyValue, "_bypass_gfw", translate("Bypass GFW"))
o.value = "<span id=\"_bypass_gfw\" style=\"line-height: 2.1em;\">%s</span>" %{translate("Collecting data...")}
o.rawhtml = true

o = s:option(Flag, "enabled", translate("Enable"))
o.default = 0
o.rmempty = false

local log_level = {"silent", "info", "warning", "error", "debug"}
o = s:option(ListValue, "log_level", translate("Log level"))
for _, v in ipairs(log_level) do o:value(v, v:upper()) end
o.default = "warning"
o.rmempty = false

o = s:option(Value, "secret", translate("Secret"))
o.default = rndLetter(10)
o.rmempty = false
o.description = translate("Secret for RESTful API")

o = s:option(Value, "api_port", translate("API Port"))
o.default = ""
o.datatype = "port"
o.rmempty = true
o.description = translate("If the value is empty disabled")

o = s:option(Value, "http_port", translate("HTTP Port"))
o.default = ""
o.datatype = "port"
o.rmempty = true
o.description = translate("If the value is empty disabled")

o = s:option(Value, "socks_port", translate("Socks5 Port"))
o.default = ""
o.datatype = "port"
o.rmempty = true
o.description = translate("If the value is empty disabled")

o = s:option(Value, "redir_port", translate("* Redir Port"))
o.default = 9892
o.datatype = "port"
o.rmempty = false
o.description = translate("Transparent proxy port")

o = s:option(Value, "dns_port", translate("* DNS Port"))
o.default = 5333
o.datatype = "port"
o.rmempty = false
o.description = translate("Clash DNS server port")

o = s:option(DynamicList, "exclude_hosts", translate("Exclude"))
o.rmempty = true
o.description = translate("Exclude hosts, not use proxy")

o = s:option(Value, "tpl_url", translate("Template url"))
o.description = translate("")
o.default = "https://cdn.jsdelivr.net/gh/antonchen/clash-for-openwrt/clash.tpl"
o.rmempty = true

o = s:option(Button, "restart", translate("Restart"))
o.inputstyle = "reload"
o.write = function()
    os.execute("sleep 5;/etc/init.d/clash restart >/dev/null 2>&1 &")
    http.redirect(dispatcher.build_url("admin", "services", "clash"))
end

m:section(SimpleSection).template  = "clash/status"

--- Apply
local apply = http.formvalue("cbi.apply")
if apply then
    os.execute("sleep 5;/etc/init.d/clash restart >/dev/null 2>&1 &")
end
return m
