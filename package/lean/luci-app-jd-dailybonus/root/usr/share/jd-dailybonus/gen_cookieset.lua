#!/usr/bin/lua
local uci = require "luci.model.uci".cursor()
local json = require 'luci.jsonc'

function write_json(file, content)
    local json = require 'luci.jsonc'
    local f = assert(io.open(file, 'w'))
    f:write(json.stringify(content, 1))
    f:close()
end

local data = {
    CookieJD = uci:get('jd-dailybonus', '@global[0]', 'cookie'),
    CookieJD2 = uci:get('jd-dailybonus', '@global[0]', 'cookie2'),
    JD_DailyBonusDelay = uci:get('jd-dailybonus', '@global[0]', 'stop'),
    JD_DailyBonusTimeOut = uci:get('jd-dailybonus', '@global[0]', 'out')
}

write_json('/usr/share/jd-dailybonus/CookieSet.json', data)