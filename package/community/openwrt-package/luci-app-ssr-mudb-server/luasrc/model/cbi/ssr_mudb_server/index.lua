local appname = "ssr_mudb_server"
local jsonc = require "luci.jsonc"

a = Map(appname, translate("ShadowsocksR MuDB Server"))

t = a:section(TypedSection, "global", translate("Global Settings"))
t.anonymous = true
t.addremove = false

t:append(Template(appname .. "/status"))

e = t:option(Flag, "enable", translate("Enable"))
e.rmempty = false

e = t:option(Flag, "auto_clear_transfer", translate("Enable Auto Clear Traffic"))
e.default = 0
e.rmempty = false

e = t:option(Value, "auto_clear_transfer_time", translate("Clear Traffic Time Interval"), translate("*,*,*,*,* is Min Hour Day Mon Week"))
e.default = "0,2,1,*,*"
e:depends("auto_clear_transfer", 1)

a:append(Template(appname .. "/users"))

local apply = luci.http.formvalue("cbi.apply")
if apply then
    for key, val in pairs(luci.http.formvalue()) do
        if key:find("ssr_mudb_server_") == 1 then
            local port = key:gsub("ssr_mudb_server_",""):gsub(".enable","")
            local config_path = luci.sys.exec("echo -n $(cat /usr/share/ssr_mudb_server/userapiconfig.py | grep 'MUDB_FILE' | cut -d \"'\" -f 2)")
            local str = luci.sys.exec("cat " .. config_path) or "[]"
            local json = jsonc.parse(str)
            if port and str and json then
                for index = 1, table.maxn(json) do
                    local o = json[index]
                    if o.port == tonumber(port) then
                        json[index].enable = tonumber(val)
                        break
                    end
                end
                local f, err = io.open(config_path, "w")
                if f and err == nil then
                    f:write(jsonc.stringify(json, 1))
                    f:close()
                end
            end
        end
    end
end

return a
