#!/usr/bin/lua
local uci = require "luci.model.uci".cursor()
local json = require 'luci.jsonc'

function write_json(file, content)
    local f = assert(io.open(file, 'w'))
    f:write(json.stringify(content, 1))
    f:close()
end


local data = {
    CookiesJD = {}, 
    JD_DailyBonusDelay = uci:get('jd-dailybonus', '@global[0]', 'stop'),
    JD_DailyBonusTimeOut = uci:get('jd-dailybonus', '@global[0]', 'out')
}

for i, v in pairs( uci:get('jd-dailybonus', '@global[0]', 'Cookies') or {} ) do
    table.insert(data.CookiesJD, {["cookie"]=v})
end

for i, v in pairs( uci:get('jd-dailybonus', '@global[0]', 'jrBody') or {} ) do
    data.CookiesJD[i]["jrBody"]=v
end

data.CookiesJD = json.stringify( data.CookiesJD )

write_json('/usr/share/jd-dailybonus/CookieSet.json', data)
