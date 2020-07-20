#!/usr/bin/lua
------------------------------------------------
-- This file is converter ip to country iso code
-- @author Jerryk <jerrykuku@qq.com>
------------------------------------------------

local mm = require 'maxminddb'
local db = mm.open('/usr/share/vssr/GeoLite2-Country.mmdb')
local res = db:lookup(arg[1])
print(string.lower(res:get("country", "iso_code")))